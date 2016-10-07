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

	int syscall = ptrace(PTRACE_PEEKUSER, _pid, sizeof(long)*ORIG_RAX);
	_syscall = syscall;

	_log.Debug("SYSCALL:", syscall);

	ptrace(PTRACE_GETREGS, _pid, NULL, &regs);

	if(syscall == SYS_open)
	{
		readRemoteText(regs.rdi, _openpath, 256);
		_currentDeviceHandler = _deviceHandlerFactory.Create(_openpath, _pid);
	}
	else 
	{
		if(regs.orig_rax == SYS_ioctl || regs.orig_rax == SYS_read || regs.orig_rax == SYS_write)
		{
			long fd = (long)regs.rdi;
			_currentDeviceHandler = _deviceHandlerRegistry.Lookup(fd);
		}
		if(regs.orig_rax == SYS_mmap)
		{
			long fd = (long)regs.r8;
			_currentDeviceHandler = _deviceHandlerRegistry.Lookup(fd);
		}
	}

	if(_currentDeviceHandler!= NULL)
	{
		_currentDeviceHandler->ExecuteBefore(syscall, regs);
	}

	_exiting=!_exiting;

	return true;
}

bool ProgRuntimeHandler::SpySyscallExit()
{
	struct user_regs_struct regs;

	int syscall = ptrace(PTRACE_PEEKUSER, _pid, sizeof(long)*ORIG_RAX);

	if(syscall == SYS_open) 
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
		_log.Info("---");
	}

	_exiting=!_exiting;

	return true;
}

bool ProgRuntimeHandler::hasZero(const unsigned long data)
{
	return ((data - 0x0101010101010101) & ~data & 0x8080808080808080) != 0;
}

bool ProgRuntimeHandler::readRemoteText(long addr, char *out, int maxlen) {
	long data;
	int idx = 0;

	do
	{
		data = ptrace(PTRACE_PEEKDATA, _pid, addr+idx, 0);
		*(long *)(&out[idx]) = data;
		idx+=sizeof(long);
		if(idx >= maxlen-(int)sizeof(long))
			return false;
	} while(!hasZero((unsigned long)data));

	return true;
}


