#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <sys/uio.h>
#include "ProgRuntimeHandler.h"
#include "DeviceHandlerFactory.h"

#include <memory>
#include <sys/epoll.h>
#include <sched.h>

#include <math.h>

ProgRuntimeHandler::ProgRuntimeHandler(pid_t pid, int status, DeviceHandlerFactory &deviceHandlerFactory, std::weak_ptr<ProgRuntimeDispatcher> runtimeDispatcher): 
	_runtimeDispatcher(runtimeDispatcher), _pid(pid), _status(status), _deviceHandlerFactory(deviceHandlerFactory), _log(Log("runtime", pid))	
{
	_deviceHandlerRegistry = std::make_shared<DeviceHandlerRegistry>();
	_exiting = false;

	maskedSysCallsForDebug.emplace(SYS_open);
	maskedSysCallsForDebug.emplace(SYS_read);
	maskedSysCallsForDebug.emplace(SYS_write);
	maskedSysCallsForDebug.emplace(SYS_clock_gettime);
	maskedSysCallsForDebug.emplace(SYS_epoll_wait);
}

bool ProgRuntimeHandler::Step()
{
	if(_exiting)
		return SpySyscallExit();

	return SpySyscallEnter();
}

void pokeData(long pid, long addr, const void *dataInput, int len) 
{
	char *data = (char *)dataInput;
	int chunkSize = sizeof(long);
	int fullChunksEnd = (len/chunkSize)*chunkSize;
	int restLen = len - fullChunksEnd;

	for(int i = 0;i < fullChunksEnd;i+=chunkSize) 
	{
		ptrace(PTRACE_POKEDATA, pid, addr + i, *(long *)&data[i]);
	}

	if(restLen > 0)
	{
		char lastChunk[chunkSize];
		long lastData = ptrace(PTRACE_PEEKDATA, pid, addr + fullChunksEnd, 0);
		*(long *)(&lastChunk) = lastData;
		for(int i = 0;i < restLen;i++)
			lastChunk[i] = data[fullChunksEnd + i];
		ptrace(PTRACE_POKEDATA, pid, addr + fullChunksEnd, *(long *)(char *)&lastChunk);
	}
}

void peekData(pid_t pid, long addr, void *dataOutput, int len) {
	struct iovec local, remote;

	local.iov_base = dataOutput;
	local.iov_len = len;
	remote.iov_base = (void *)addr;
	remote.iov_len = len;

	process_vm_readv(pid, &local, 1, &remote, 1, 0);
}

long getTimeMillisec()
{
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);

	long s = (long)spec.tv_sec;
	long ms = round(spec.tv_nsec / 1000000);

	return 1000 * s + ms;
}

bool ProgRuntimeHandler::SpySyscallEnter()
{
	struct user_regs_struct regs;
	
	_currentDeviceHandler = nullptr;

	ptrace(PTRACE_GETREGS, _pid, NULL, &regs);
	_syscall = regs.orig_rax;

	auto masked = maskedSysCallsForDebug.find(_syscall);
	if(masked == maskedSysCallsForDebug.end()) {
		_log.Debug("SYSCALL:", _syscall);
	}

	if(_syscall == SYS_open)
	{
		ReadRemoteText(regs.rdi, _openpath, sizeof(_openpath));
		_currentDeviceHandler = _deviceHandlerFactory.Create(_openpath, _pid);
	}
	else if(_syscall == SYS_stat || _syscall == SYS_lstat)
	{
		// For ev devices this can be called before open. evdev uses this
		// to get the major/minor types. WSL doesn't have the /sys/dev/char
		// path so we have to emulate it with the "openat" calls that follow
		ReadRemoteText(regs.rdi, _openpath, sizeof(_openpath));
		_currentDeviceHandler = _deviceHandlerFactory.Create(_openpath, _pid);
	}
	else if(_syscall == SYS_fstat) {
		long fd = (long)regs.rdi;
		_currentDeviceHandler = _deviceHandlerRegistry->Lookup(fd);
	}
	else if(_syscall == SYS_readlinkat)
	{
		ReadRemoteText(regs.rsi, _openpath, sizeof(_openpath));
		_currentDeviceHandler = _deviceHandlerFactory.Create(_openpath, _pid);
	}
	else if(_syscall == SYS_openat)
	{
		ReadRemoteText(regs.rsi, _openpath, sizeof(_openpath));
		_currentDeviceHandler = _deviceHandlerFactory.Create(_openpath, _pid);
	}
	else 
	{
		if(_syscall == SYS_ioctl || _syscall == SYS_read || _syscall == SYS_write)
		{
			long fd = (long)regs.rdi;
			_currentDeviceHandler = _deviceHandlerRegistry->Lookup(fd);
		}
		else if(_syscall == SYS_mmap)
		{
			long fd = (long)regs.r8;
			_currentDeviceHandler = _deviceHandlerRegistry->Lookup(fd);
		}
		else if(_syscall == SYS_epoll_ctl)
		{
			long fd = (long)regs.rdx;
			_log.Debug("epoll", fd);
			_currentDeviceHandler = _deviceHandlerRegistry->Lookup(fd);
			_log.Debug("epoll", _currentDeviceHandler);

			auto& fdmap = epollList[regs.rdi];

			switch(regs.rsi) {
				case EPOLL_CTL_ADD:
					{
						auto eventData = std::make_shared<EventData>();
						eventData->lastAccess = 0;
						peekData(_pid, regs.rcx, &eventData->event, sizeof(epoll_event));
						fdmap.emplace(regs.rdx, eventData);
					}
					_log.Debug("-= add =- epfd:", regs.rdi, " for fd:", regs.rdx);
					break;
				case EPOLL_CTL_DEL:
					{
						auto localSetIt = fdmap.find(regs.rdx);
						if(localSetIt != fdmap.end()) {
							fdmap.erase(localSetIt);
						}
					}
					_log.Debug("-= del =- epfd:", regs.rdi, " for fd:", regs.rdx);
					break;
				case EPOLL_CTL_MOD:
					_log.Debug("-= mod =- epfd:", regs.rdi, " for fd:", regs.rdx);
					break;
				default:
					_log.Debug("-= unknown ", regs.rsi, " =- epfd:", regs.rdi, " for fd:", regs.rdx);
					break;
			}
		}
		else if(_syscall == SYS_close)
		{
			long fd = (long)regs.rdi;
			_log.Debug("Closing fd:", fd);
			_currentDeviceHandler = NULL;
			_deviceHandlerRegistry->Unregister(fd);
		}
	}

	if(_currentDeviceHandler!= NULL)
	{
		_currentDeviceHandler->ExecuteBefore(_pid, _syscall, regs);
	}

	_exiting=!_exiting;

	return true;
}

bool ProgRuntimeHandler::SpySyscallExit()
{
	struct user_regs_struct regs;

	int syscall = ptrace(PTRACE_PEEKUSER, _pid, sizeof(long)*ORIG_RAX);
	ptrace(PTRACE_GETREGS, _pid, NULL, &regs);

	if(syscall == SYS_clone)
	{
		_log.Debug("SYS_clone: ", _pid, " to ", regs.rax);
		_log.Debug("    cloning files: ", regs.rdx & CLONE_FILES);
		if((regs.rdx & CLONE_FILES) == 0) {
			// Only move the current file handles into the other process
			// Get the device _deviceHandlerRegistry of the other process
			auto dispatcher = _runtimeDispatcher.lock();
			if(dispatcher) {
				bool res = dispatcher->CloneHandlesToPid(regs.rax, _deviceHandlerRegistry);
				if(!res) {
					_log.Error("Failed CloneHandlesToPid.");
				}
			}
		} else {
			// Maintain a hard file handle connect permanently
			_log.Error("Process is cloning files, this is not supported yet.");
		}
	}

	if(syscall == SYS_open || syscall == SYS_openat) 
	{
		ptrace(PTRACE_GETREGS, _pid, NULL, &regs);
		long fd = (long)regs.rax;
		_log.Debug("Open result:", _openpath, "-", fd);	
		if(fd >= 0)
		{
			if(_currentDeviceHandler!= NULL)
			{
				_deviceHandlerRegistry->Register(fd, _currentDeviceHandler);
			}
		}
		else
		{
			if(_currentDeviceHandler!= NULL)
			{
				_currentDeviceHandler = nullptr;
			}
		}
	}
	if(_currentDeviceHandler!= NULL)
	{
		ptrace(PTRACE_GETREGS, _pid, NULL, &regs);
		_currentDeviceHandler->ExecuteAfter(_pid, syscall, regs);
		//_log.Debug("---");
	}

	if(syscall == SYS_epoll_wait) {
		uint64_t currentEventCount = regs.rax;
		uint64_t epfd = regs.rdi;

		uint64_t maxEvents = regs.rdx;

		_log.Debug("Epoll wait on fd:", epfd, " event count", currentEventCount, " max events", maxEvents);

		uint64_t eventArray = regs.rsi;
		auto fdset = epollList.find(epfd);
		currentEventCount = 0;
		auto t = getTimeMillisec();
		if(fdset != epollList.end()) {
			for(auto& evt : fdset->second) {
				auto fd = evt.first;
				auto devptr = _deviceHandlerRegistry->Lookup(fd);
				if((devptr != nullptr) && (currentEventCount < maxEvents)) {
					if(evt.second->lastAccess + 50 < t) {
						_log.Debug("  -- adding event for ", fd);
						struct epoll_event anEvent{};
						anEvent.events = EPOLLIN;
						anEvent.data.fd = fd;
						anEvent.data.ptr = evt.second->event.data.ptr;
						anEvent.data.u32 = evt.second->event.data.u32;
						anEvent.data.u64 = evt.second->event.data.u64;
						evt.second->lastAccess = t;
						pokeData(_pid, eventArray + (sizeof(struct epoll_event) * 
								currentEventCount), &anEvent, sizeof(anEvent));
						++currentEventCount;
					}
				}
			}
		}
		if(currentEventCount > 0)
		{
			_log.Debug("  Custom handled event count", currentEventCount);
			regs.rax = currentEventCount;

			ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
		}
	}


	_exiting=!_exiting;

	return true;
}

bool ProgRuntimeHandler::HasZero(const unsigned long data)
{
	return ((data - 0x0101010101010101) & ~data & 0x8080808080808080) != 0;
}

bool ProgRuntimeHandler::ReadRemoteText(long addr, char *out, int maxlen) {
	long data;
	int idx = 0;

	do
	{
		data = ptrace(PTRACE_PEEKDATA, _pid, addr+idx, 0);
		*(long *)(&out[idx]) = data;
		idx+=sizeof(long);
		if(idx >= maxlen-(int)sizeof(long))
			return false;
	} while(!HasZero((unsigned long)data));
	out[maxlen-1] = '\0';

	return true;
}

bool ProgRuntimeHandler::AcceptHandlesFromOtherProcess(const HandleMap& handleMap)
{
	_log.Debug(_pid, "received", handleMap.size(), "handles from process");
	return _deviceHandlerRegistry->RegisterMap(handleMap);
}

