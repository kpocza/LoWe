#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "EvdevDeviceHandler.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <asm-generic/ioctls.h>

EvdevDeviceHandler::EvdevDeviceHandler(const pid_t pid, const string openpath,
	const string logName, const string exposerId): CommunicatingDeviceHandler(pid, openpath, logName, exposerId)
{
	_isEnabled = false;
	_log.Info("Path:", _openpath);
}

string EvdevDeviceHandler::GetFixupScript() const
{
	if(!HasPermissions())
		return GetFixupScriptCore();

	return "";
}

void EvdevDeviceHandler::ExecuteBefore(const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;
	if(syscall == SYS_ioctl)
	{
		_log.Info("-= Before ioctl =-");
		_log.Debug("Ioctl regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		_ioctlop = regs.rsi;
		_ioctladdr = regs.rdx;
		if(_ioctlop == EVIOCGRAB)
		{
			_log.Info("EVIOCGRAB");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(0, sizeof(ev)))
		{
			_log.Info("EVIOCGBIT_0");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(EV_REL, sizeof(relbits)))
		{
			_log.Info("EVIOCGBIT_EV_REL");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(EV_ABS, sizeof(absbits)))
		{
			_log.Info("EVIOCGBIT_EV_ABS");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(EV_LED, sizeof(ledbits)))
		{
			_log.Info("EVIOCGBIT_EV_LED");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(EV_KEY, sizeof(keybits)))
		{
			_log.Info("EVIOCGBIT_EV_KEY");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(EV_SW, sizeof(swbits)))
		{
			_log.Info("EVIOCGBIT_EV_SW");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(EV_MSC, sizeof(mscbits)))
		{
			_log.Info("EVIOCGBIT_EV_MSC");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(EV_FF, sizeof(ffbits)))
		{
			_log.Info("EVIOCGBIT_EV_FF");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGBIT(EV_SND, sizeof(sndbits)))
		{
			_log.Info("EVIOCGBIT_EV_SND");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGNAME(255))
		{
			_log.Info("EVIOCGNAME_255");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGPHYS(255))
		{
			_log.Info("EVIOCGPHYS_255");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGUNIQ(255))
		{
			_log.Info("EVIOCGUNIQ_255");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGID)
		{
			_log.Info("EVIOCGID");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGVERSION)
		{
			_log.Info("EVIOCGVERSION");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGPROP(8))
		{
			_log.Info("EVIOCGPROP(8)");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGKEY(sizeof(keystate)))
		{
			_log.Info("EVIOCGKEY");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGLED(sizeof(ledstate)))
		{
			_log.Info("EVIOCGLED");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGSW(sizeof(swstate)))
		{
			_log.Info("EVIOCGSW");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGSND(sizeof(sndstate)))
		{
			_log.Info("EVIOCGSND");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGABS(ABS_X))
		{
			_log.Info("EVIOCGABS_X");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == EVIOCGABS(ABS_Y))
		{
			_log.Info("EVIOCGABS_Y");
			regs.orig_rax = -1;
		}
		else if(_ioctlop == TCFLSH)
		{
			_log.Info("TCFLSH");
			regs.orig_rax = -1;
		}
		else
		{
			_log.Error("unknown ioctl op ", _ioctlop);
		}
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(syscall == SYS_read)
	{
		_log.Debug("-= Before read =-");
		_log.Debug("Read regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);

		_readaddr = regs.rsi;
		_readlen = regs.rdx;

		_log.Debug("Read size:", _readlen);

		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(syscall!= SYS_open)
	{
		_log.Info("Other syscall:", syscall);
	}
}

void EvdevDeviceHandler::ExecuteAfter(const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	if(_syscallbefore == SYS_open)
	{
	}
	if(_syscallbefore == SYS_ioctl) 
	{
		_log.Info("-= After ioctl =-");
		if(_ioctlop == EVIOCGRAB)
		{
			_log.Info("EVIOCGRAB");
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(0, sizeof(ev)))
		{
			_log.Info("EVIOCGBIT_0");
			long ev = GetEv();
			PokeData(_ioctladdr, &ev, sizeof(ev));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_REL, sizeof(relbits)))
		{
			_log.Info("EVIOCGBIT_EV_REL");
			SetRelBits();
			PokeData(_ioctladdr, relbits, sizeof(relbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_ABS, sizeof(absbits)))
		{
			_log.Info("EVIOCGBIT_EV_ABS");
			SetAbsBits();
			PokeData(_ioctladdr, absbits, sizeof(absbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_LED, sizeof(ledbits)))
		{
			_log.Info("EVIOCGBIT_EV_LED");
			PokeData(_ioctladdr, ledbits, sizeof(ledbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_KEY, sizeof(keybits)))
		{
			_log.Info("EVIOCGBIT_EV_KEY");
			SetKeyBits();
			PokeData(_ioctladdr, keybits, sizeof(keybits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_SW, sizeof(swbits)))
		{
			_log.Info("EVIOCGBIT_EV_SW");
			PokeData(_ioctladdr, swbits, sizeof(swbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_MSC, sizeof(mscbits)))
		{
			_log.Info("EVIOCGBIT_EV_MSC");
			PokeData(_ioctladdr, mscbits, sizeof(mscbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_FF, sizeof(ffbits)))
		{
			_log.Info("EVIOCGBIT_EV_FF");
			PokeData(_ioctladdr, ffbits, sizeof(ffbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_SND, sizeof(sndbits)))
		{
			_log.Info("EVIOCGBIT_EV_SND");
			PokeData(_ioctladdr, sndbits, sizeof(sndbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGNAME(255))
		{
			_log.Info("EVIOCGNAME_255");
			const char *name = GetName().c_str();
			PokeData(_ioctladdr, (void *)name, 6);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGPHYS(255))
		{
			_log.Info("EVIOCGPHYS_255");
			const char *name="here";
			PokeData(_ioctladdr, (void *)name, 5);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGUNIQ(255))
		{
			_log.Info("EVIOCGUNIQ_255");
			const char *name="12345";
			PokeData(_ioctladdr, (void *)name, 6);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGID)
		{
			_log.Info("EVIOCGID");
			long val=0;
			PokeData(_ioctladdr, (void *)&val, 8);
			regs.rax = 0;
		}	
		else if(_ioctlop == EVIOCGVERSION)
		{
			_log.Info("EVIOCGVERSION");
			long val=0;
			PokeData(_ioctladdr, (void *)&val, 4);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGPROP(8))
		{
			_log.Info("EVIOCGPROP(8)");
			long val=0;
			PokeData(_ioctladdr, (void *)&val, 8);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGKEY(sizeof(keystate)))
		{
			_log.Info("EVIOCGKEY");
			PokeData(_ioctladdr, (void *)keystate, sizeof(keystate));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGLED(sizeof(ledstate)))
		{
			_log.Info("EVIOCGLED");
			PokeData(_ioctladdr, (void *)&ledstate, sizeof(ledstate));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGSW(sizeof(swstate)))
		{
			_log.Info("EVIOCGSW");
			PokeData(_ioctladdr, (void *)&swstate, sizeof(swstate));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGSND(sizeof(sndstate)))
		{
			_log.Info("EVIOCGSND");
			PokeData(_ioctladdr, (void *)&sndstate, sizeof(sndstate));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGABS(ABS_X))
		{
			_log.Info("EVIOCGABS_X");
			struct input_absinfo a;
			a.value = 1280/2;
			a.minimum = 0;
			a.maximum = 1280;
			a.fuzz = 0;
			a.flat = 0;
			a.resolution = 1;
			PokeData(_ioctladdr, (void *)&a, sizeof(a));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGABS(ABS_Y))
		{
			_log.Info("EVIOCGABS_Y");
			struct input_absinfo a;
			a.value = 720/2;
			a.minimum = 0;
			a.maximum = 720;
			a.fuzz = 0;
			a.flat = 0;
			a.resolution = 1;
			PokeData(_ioctladdr, (void *)&a, sizeof(a));
			regs.rax = 0;
		}
		else if(_ioctlop == TCFLSH)
		{
			_log.Info("TCFLSH");
			PreEnabling();
			
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
			regs.rax = 0;
		}

		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(_syscallbefore == SYS_read)
	{
		_log.Debug("-= After read =-");

		ReadLogic(regs);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

