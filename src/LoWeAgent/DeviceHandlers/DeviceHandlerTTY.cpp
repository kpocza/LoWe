#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerTTY.h"
#include <string.h>
#include <unistd.h>
#include <asm-generic/ioctls.h>

DeviceHandlerTTY::DeviceHandlerTTY(const pid_t pid, const string openpath): DeviceHandler(pid, openpath, "tty", "")
{
	_log.Info("Path:", _openpath);

	memset(&_vt_stat, 0, sizeof(_vt_stat));
	memset(&_vt_mode, 0, sizeof(_vt_mode));
	_kdmode = KD_TEXT;
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
	if(_openpath == "/dev/tty99" && !HasPermissions())
		return GetFixupScriptCore();
		
	return "";
}

void DeviceHandlerTTY::ExecuteBefore(const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;

	if(syscall == SYS_open)
	{
		_log.Info("-= Before open =-");
		if(_openpath == "/dev/tty0")
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
		else if(_ioctlop == KDGETMODE)
		{
			_log.Info("KDGETMODE");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == KDSKBMODE)
		{
			_log.Info("KDSKBMODE");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == TIOCLINUX)
		{
			_log.Info("TIOCLINUX");
			regs.orig_rax = -1;
		}
		else
		{
			_log.Info("Unknown ioctl:", _ioctlop);
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
		_log.Info("-= After ioctl  =-");
		if(_ioctlop == VT_GETSTATE)
		{
			_log.Info("VT_GETSTATE");
			PokeData(_ioctladdr, &_vt_stat, sizeof(_vt_stat));
			regs.rax = 0;
		}
		else if(_ioctlop == VT_OPENQRY)
		{
			_log.Info("VT_OPENQRY");
			int data;
			data=99;
			PokeData(_ioctladdr, &data, 4);

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
			PokeData(_ioctladdr, &_vt_mode, sizeof(_vt_mode));
			regs.rax = 0;
		}
		else if(_ioctlop == VT_SETMODE)
		{
			_log.Info("VT_SETMODE");
			PeekData(_ioctladdr, &_vt_mode, sizeof(_vt_mode));
			regs.rax = 0;
		}
		else if(_ioctlop == KDSETMODE)
		{
			_log.Info("KDSETMODE");
			PeekData(_ioctladdr, &_kdmode, sizeof(_kdmode));
			regs.rax = 0;
		}
		else if(_ioctlop == KDGETMODE)
		{
			_log.Info("KDGETMODE");
			PokeData(_ioctladdr, &_kdmode, sizeof(_kdmode));
			regs.rax = 0;
		}
		else if(_ioctlop == KDSKBMODE)
		{
			_log.Info("KDSKBMODE");
			PeekData(_ioctladdr, &_kbmode, sizeof(_kbmode));
			regs.rax = 0;
		}
		else if(_ioctlop == TIOCLINUX)
		{
			_log.Info("TIOCLINUX");
			regs.rax = 0;
		}
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

