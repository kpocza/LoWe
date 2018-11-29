#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include "ProgRuntimeHandler.h"
#include "DeviceHandlerFactory.h"

ProgRuntimeHandler::ProgRuntimeHandler(pid_t pid, int status, DeviceHandlerFactory &deviceHandlerFactory): 
	_pid(pid), _status(status), _deviceHandlerFactory(deviceHandlerFactory), _log(Log("runtime", pid))
{
	_exiting = false;
}

bool ProgRuntimeHandler::Step()
{
	if(_exiting)
		return SpySyscallExit();

	return SpySyscallEnter();
}

bool ProgRuntimeHandler::SpySyscallEnter()
{
	struct user_regs_struct regs;
	
	_currentDeviceHandler = NULL;

	ptrace(PTRACE_GETREGS, _pid, NULL, &regs);
	_syscall = regs.orig_rax;

	_log.Debug("SYSCALL:", _syscall);

	if(_syscall == SYS_open)
	{
		ReadRemoteText(regs.rdi, _openpath, sizeof(_openpath));
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
			_currentDeviceHandler = _deviceHandlerRegistry.Lookup(fd);
		}
		else if(_syscall == SYS_mmap)
		{
			long fd = (long)regs.r8;
			_currentDeviceHandler = _deviceHandlerRegistry.Lookup(fd);
		}
		else if(_syscall == SYS_epoll_ctl)
		{
			long fd = (long)regs.rdx;
			_log.Debug("epoll", fd);
			_currentDeviceHandler = _deviceHandlerRegistry.Lookup(fd);
			_log.Debug("epoll", _currentDeviceHandler);
		}
		else if(_syscall == SYS_close)
		{
			long fd = (long)regs.rdi;
			_log.Debug("Closing fd:", fd);
			_currentDeviceHandler = NULL;
			_deviceHandlerRegistry.Unregister(fd);
		}
	}

	if(_currentDeviceHandler!= NULL)
	{
		_currentDeviceHandler->ExecuteBefore(_syscall, regs);
	}

	_exiting=!_exiting;

	return true;
}

bool ProgRuntimeHandler::SpySyscallExit()
{
	struct user_regs_struct regs;

	int syscall = ptrace(PTRACE_PEEKUSER, _pid, sizeof(long)*ORIG_RAX);

	if(syscall == SYS_clone)
	{
		ptrace(PTRACE_GETREGS, _pid, NULL, &regs);
		_deviceHandlerRegistry.AddProcessRelationship(regs.rax, _pid);
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
				_deviceHandlerRegistry.Register(fd, _currentDeviceHandler);
			}
		}
		else
		{
			if(_currentDeviceHandler!= NULL)
			{
				delete _currentDeviceHandler;
				_currentDeviceHandler = NULL;
			}
		}
	}
	if(_currentDeviceHandler!= NULL)
	{
		ptrace(PTRACE_GETREGS, _pid, NULL, &regs);
		_currentDeviceHandler->ExecuteAfter(syscall, regs);
		_log.Debug("---");
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


