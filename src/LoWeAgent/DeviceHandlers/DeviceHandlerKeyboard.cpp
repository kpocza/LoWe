#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerKeyboard.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

DeviceHandlerKeyboard::DeviceHandlerKeyboard(const pid_t pid, const string openpath): 
	CommunicatingDeviceHandler(pid, openpath, "kbd", "KEYB")
{
	_log.Info("Path:", _openpath);
	_isEnabled = false;
	_lastMillisec = 0;
}

string DeviceHandlerKeyboard::GetFixupScript() const
{
	if(!HasPermissions())
		return GetFixupScriptCore();

	return "";
}

void DeviceHandlerKeyboard::ExecuteBefore(const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;
	if(syscall == SYS_ioctl)
	{
		_log.Info("-= Before ioctl =-");
		_log.Debug("Ioctl regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		_ioctlop = regs.rsi;
		_ioctladdr = regs.rdx;
		_log.Error("unknown ioctl op ", _ioctlop);
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(syscall == SYS_read)
	{
		if(!_isEnabled)
			_log.Info("-= Before read =-");
		else
			_log.Debug("-= Before read =-");

		_log.Debug("Read regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);

		_readaddr = regs.rsi;
		_readlen = regs.rdx;

		if(!_isEnabled)
			_log.Info("Read size:", _readlen);
		else
			_log.Debug("Read size:", _readlen);

		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(syscall!= SYS_open)
	{
		_log.Info("Other syscall:", syscall);
	}
}

void DeviceHandlerKeyboard::ExecuteAfter(const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	if(_syscallbefore == SYS_open)
	{
		if(_socketCommunicator.Open("127.0.0.1", GetPort()))
		{
			_log.Info("socket opened");
			SendOpcode("INIT");
			_isEnabled = true;
		}
		else 
		{
			_log.Error("socket open failed");
			_isEnabled = false;
		}
	}
	else if(_syscallbefore == SYS_read)
	{
		int size = 0;
		if(_isEnabled)
		{
			long now = GetTimeMillisec();
			if(now - _lastMillisec > 100)
			{
				_lastMillisec = now;
				SendOpcode("READ");
				_log.Debug("-= After read =-");
				_socketCommunicator.Send((char *)&_readlen, 4);
				_socketCommunicator.Recv((char *)&size, 4);
				if(size > 0)
				{
					char results[size];
					_socketCommunicator.Recv((char *)&results, size);
					PokeData(_readaddr, &results, size);
				}
			}
		}
		regs.rax = size;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);

		if(!_isEnabled)
			_log.Info("Read size:", size);
		else
			_log.Debug("Read size:", size);
	}
	else if(_syscallbefore == SYS_ioctl) 
	{
		_log.Info("-= After ioctl =-");
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

