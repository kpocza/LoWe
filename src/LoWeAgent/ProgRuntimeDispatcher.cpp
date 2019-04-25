#include <sys/ptrace.h>
#include <errno.h>
#include <sys/wait.h>
#include "ProgRuntimeDispatcher.h"

ProgRuntimeDispatcher::ProgRuntimeDispatcher(DeviceHandlerFactory &deviceHandlerFactory):
	_deviceHandlerFactory(deviceHandlerFactory), _log(Log("dispatcher"))
{
	_numberOfProcesses = 0;
}

bool ProgRuntimeDispatcher::Init(pid_t pid, bool isExec)
{
	if(!isExec)
	{
		int res = ptrace(PTRACE_ATTACH, pid);
		if(res) 
		{
			_log.Error("Cannot attach process. Result code:", res, "errno:", errno);
			return false;
		}
	}

	int options = PTRACE_O_TRACESYSGOOD|PTRACE_O_TRACECLONE|PTRACE_O_TRACEFORK|PTRACE_O_TRACEVFORK;
	if(isExec)
		options|=PTRACE_O_TRACEEXEC;

	ptrace(PTRACE_SETOPTIONS, pid, 0, options);

	ptrace(PTRACE_SYSCALL, pid, 0, 0);

	return true;
}

bool ProgRuntimeDispatcher::Spin()
{
	while(Step());

	return true;
}

bool ProgRuntimeDispatcher::CloneHandlesToPid(pid_t destPid, std::shared_ptr<DeviceHandlerRegistry> registry)
{
	auto item = _runtimeInfo.find(destPid);
	if(item == _runtimeInfo.end()) {
		return false;
	}

	return registry->OneTimeDuplicateHandles(item->second);
}

std::shared_ptr<ProgRuntimeHandler> ProgRuntimeDispatcher::GetOrAdd(pid_t pid, int status)
{
	auto item = _runtimeInfo.find(pid);

	if(item!= _runtimeInfo.end()) {
		return item->second;
	} else {
		_log.Debug("ProgRuntimeDispatcher GetOrAdd:", pid);
	}

	auto progRuntimeHandler = std::make_shared<ProgRuntimeHandler>(pid, status, _deviceHandlerFactory, shared_from_this()); 
	_runtimeInfo.emplace(pid, progRuntimeHandler);

	_numberOfProcesses++;
	ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD|
		PTRACE_O_TRACECLONE|PTRACE_O_TRACEFORK|PTRACE_O_TRACEVFORK);

	return progRuntimeHandler;
}

void ProgRuntimeDispatcher::Drop(pid_t pid)
{
	auto item = _runtimeInfo.find(pid);

	if(item!= _runtimeInfo.end())
	{
		_runtimeInfo.erase(item);
		_numberOfProcesses--;
	}
}

bool ProgRuntimeDispatcher::Step()
{
	int status;
	pid_t pid = wait4(-1, &status, __WALL, NULL);
	int wait_errno = errno;

	if(pid < 0) 
	{
		if(wait_errno == EINTR)
			return true;

		if(_numberOfProcesses == 0 && wait_errno == ECHILD)
			return false;
	}
	
	auto progRuntimeHandler = GetOrAdd(pid, status);

	unsigned int event = (unsigned int)status>>16;

	if(WIFSIGNALED(status))
	{
		Drop(pid);
		return true;
	}

	if(WIFEXITED(status))
	{
		Drop(pid);
		return true;
	}

	if(!WIFSTOPPED(status))
	{
		Drop(pid);
		return true;
	}

	int sig = WSTOPSIG(status);
	bool skip = false;

	if(event!= 0)
	{
		sig = 0;
		skip = true;
	}
	if(!skip && sig != (SIGTRAP | 0x80))
	{
		skip = true;
	}

	if(!skip)
	{
		if(!progRuntimeHandler->Step())
		{
			return false;
		}
	}

	errno = 0;
	ptrace(PTRACE_SYSCALL, pid, 0, sig);
	if(!errno || errno == ESRCH)
	{
		return true;
	}

	return false;
}

