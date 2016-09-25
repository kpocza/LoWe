#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerTTY.h"
#include <string.h>
#include <unistd.h>

DeviceHandlerTTY::DeviceHandlerTTY(const pid_t pid, const char *openpath): DeviceHandler(pid, openpath, Log("tty"))
{
	_log.Info("Path:", _openpath);

	memset(&_vt_stat, 0, sizeof(_vt_stat));
	memset(&_vt_mode, 0, sizeof(_vt_mode));
	_kdmode = 0;
	_kbmode = 0;
}

bool DeviceHandlerTTY::IsDeviceAvailable()
{
	if(_openpath == "/dev/tty0")
		return true;

	return DeviceHandler::IsDeviceAvailable();
}


string DeviceHandlerTTY::GetFixupScript() const
{
	return "";
}

void DeviceHandlerTTY::ExecuteBefore(const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;

	if(syscall == SYS_open)
	{
		_log.Info("-= Before open =-");
		if(!strcmp(_openpath.c_str(), "/dev/tty0"))
		{
			char tty[16];
			PeekData(regs.rdi, tty, 16);
			// /dev/tty0 -> /dev/tty
			tty[8] = 0;
			PokeData(regs.rdi, tty, 16);
			_log.Info("/dev/tty0 -> /dev/tty");
		}	
	}
	else if(syscall == SYS_ioctl)
	{
		_log.Info("-= Before ioctl  =-");
		_log.Debug("Ioctl regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		_ioctlop = regs.rsi;
		_ioctladdr = regs.rdx;

		if(_ioctlop == VT_GETSTATE)
		{
			_log.Info("VT_GETSTATE");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == VT_OPENQRY)
		{
			_log.Info("VT_OPENQRY");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == VT_ACTIVATE)
		{
			_log.Info("VT_ACTIVATE");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == VT_WAITACTIVE)
		{
			_log.Info("VT_WAITACTIVE");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == VT_GETMODE)
		{
			_log.Info("VT_GETMODE");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == VT_SETMODE)
		{
			_log.Info("VT_SETMODE");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == KDSETMODE)
		{
			_log.Info("KDSETMODE");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == KDSKBMODE)
		{
			_log.Info("KDSKBMODE");
			regs.orig_rax = -1;
		}
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	} 
	else
	{
		_log.Info("Other syscall:", syscall);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

void DeviceHandlerTTY::ExecuteAfter(const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	if(_syscallbefore == SYS_ioctl) 
	{
		_log.Info("-= After open  =-");
		if(_ioctlop == VT_GETSTATE)
		{
			_log.Info("VT_GETSTATE");
			PokeData(_ioctladdr, (char *)&_vt_stat, sizeof(_vt_stat));
			regs.rax = 0;
		}
		else if(_ioctlop == VT_OPENQRY)
		{
			_log.Info("VT_OPENQRY");
			int data[2];
			PeekData(_ioctladdr, (char *)&data, 8);
			data[0]=4;
			PokeData(_ioctladdr, (char *)&data, 8);

			regs.rax = 0;
		}
		else if(_ioctlop == VT_ACTIVATE)
		{
			_log.Info("VT_ACTIVATE");
			regs.rax = 0;
		}
		else if(_ioctlop == VT_WAITACTIVE)
		{
			_log.Info("VT_WAITACTIVE");
			regs.rax = 0;
		}
		else if(_ioctlop == VT_GETMODE)
		{
			_log.Info("VT_GETMODE");
			PokeData(_ioctladdr, (char *)&_vt_mode, sizeof(_vt_mode));
			regs.rax = 0;
		}
		else if(_ioctlop == VT_SETMODE)
		{
			_log.Info("VT_SETMODE");
			PeekData(_ioctladdr, (char *)&_vt_mode, sizeof(_vt_mode));
			regs.rax = 0;
		}
		else if(_ioctlop == KDSETMODE)
		{
			_log.Info("KDSETMODE");
			PeekData(_ioctladdr, (char *)&_kdmode, sizeof(_kdmode));
			regs.rax = 0;
		}
		else if(_ioctlop == KDSKBMODE)
		{
			_log.Info("KDSKBMODE");
			PeekData(_ioctladdr, (char *)&_kbmode, sizeof(_kbmode));
			regs.rax = 0;
		}
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}
