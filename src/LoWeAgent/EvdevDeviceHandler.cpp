#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include "EvdevDeviceHandler.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <asm-generic/ioctls.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>

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

void EvdevDeviceHandler::ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs)
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
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else if(syscall == SYS_read)
	{
		//_log.Debug("-= Before read =-");
		//_log.Debug("Read regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);

		_readaddr = regs.rsi;
		_readlen = regs.rdx;

		//_log.Debug("Read size:", _readlen);

		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else if(syscall == SYS_epoll_ctl)
	{
		_log.Debug("-= Before epoll ctl =-");
		switch(regs.rsi) {
			case EPOLL_CTL_ADD:
				_log.Debug("-= add =- epfd:", regs.rdi, " for fd:", regs.rdx);
				break;
			case EPOLL_CTL_DEL:
				_log.Debug("-= del =- epfd:", regs.rdi, " for fd:", regs.rdx);
				break;
			default:
				_log.Debug("-= unknown =- epfd:", regs.rdi, " for fd:", regs.rdx);
				break;
		}
	}
	else if(syscall == SYS_stat || syscall == SYS_fstat || syscall == SYS_lstat){
		_log.Debug("-= Before stat =-");
		_stataddr = regs.rsi;
		regs.orig_rax = -1;
	}

	else if(syscall!= SYS_open)
	{
		_log.Info("Other syscall:", syscall);
	}
}

void EvdevDeviceHandler::ExecuteAfter(pid_t pid, const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

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
			PokeData(pid, _ioctladdr, &ev, sizeof(ev));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_REL, sizeof(relbits)))
		{
			_log.Info("EVIOCGBIT_EV_REL");
			SetRelBits();
			PokeData(pid, _ioctladdr, relbits, sizeof(relbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_ABS, sizeof(absbits)))
		{
			_log.Info("EVIOCGBIT_EV_ABS");
			SetAbsBits();
			PokeData(pid, _ioctladdr, absbits, sizeof(absbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_LED, sizeof(ledbits)))
		{
			_log.Info("EVIOCGBIT_EV_LED");
			PokeData(pid, _ioctladdr, ledbits, sizeof(ledbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_KEY, sizeof(keybits)))
		{
			_log.Info("EVIOCGBIT_EV_KEY");
			SetKeyBits();
			PokeData(pid, _ioctladdr, keybits, sizeof(keybits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_SW, sizeof(swbits)))
		{
			_log.Info("EVIOCGBIT_EV_SW");
			PokeData(pid, _ioctladdr, swbits, sizeof(swbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_MSC, sizeof(mscbits)))
		{
			_log.Info("EVIOCGBIT_EV_MSC");
			PokeData(pid, _ioctladdr, mscbits, sizeof(mscbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_FF, sizeof(ffbits)))
		{
			_log.Info("EVIOCGBIT_EV_FF");
			PokeData(pid, _ioctladdr, ffbits, sizeof(ffbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGBIT(EV_SND, sizeof(sndbits)))
		{
			_log.Info("EVIOCGBIT_EV_SND");
			PokeData(pid, _ioctladdr, sndbits, sizeof(sndbits));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGNAME(255))
		{
			_log.Info("EVIOCGNAME_255");
			const char *name = GetName().c_str();
			PokeData(pid, _ioctladdr, (void *)name, 6);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGPHYS(255))
		{
			_log.Info("EVIOCGPHYS_255");
			const char *name="here";
			PokeData(pid, _ioctladdr, (void *)name, 5);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGUNIQ(255))
		{
			_log.Info("EVIOCGUNIQ_255");
			const char *name="12345";
			PokeData(pid, _ioctladdr, (void *)name, 6);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGID)
		{
			_log.Info("EVIOCGID");
			long val=0;
			PokeData(pid, _ioctladdr, (void *)&val, 8);
			regs.rax = 0;
		}	
		else if(_ioctlop == EVIOCGVERSION)
		{
			_log.Info("EVIOCGVERSION");
			long val=0;
			PokeData(pid, _ioctladdr, (void *)&val, 4);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGPROP(8))
		{
			_log.Info("EVIOCGPROP(8)");
			long val=0;
			PokeData(pid, _ioctladdr, (void *)&val, 8);
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGKEY(sizeof(keystate)))
		{
			_log.Info("EVIOCGKEY");
			PokeData(pid, _ioctladdr, (void *)keystate, sizeof(keystate));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGLED(sizeof(ledstate)))
		{
			_log.Info("EVIOCGLED");
			PokeData(pid, _ioctladdr, (void *)&ledstate, sizeof(ledstate));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGSW(sizeof(swstate)))
		{
			_log.Info("EVIOCGSW");
			PokeData(pid, _ioctladdr, (void *)&swstate, sizeof(swstate));
			regs.rax = 0;
		}
		else if(_ioctlop == EVIOCGSND(sizeof(sndstate)))
		{
			_log.Info("EVIOCGSND");
			PokeData(pid, _ioctladdr, (void *)&sndstate, sizeof(sndstate));
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
			PokeData(pid, _ioctladdr, (void *)&a, sizeof(a));
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
			PokeData(pid, _ioctladdr, (void *)&a, sizeof(a));
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

		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else if(_syscallbefore == SYS_read)
	{
		//_log.Debug("-= After read =-");

		ReadLogic(pid, regs);
	}
	else if(_syscallbefore == SYS_stat || _syscallbefore == SYS_fstat || _syscallbefore == SYS_lstat)
	{
		dev_t devnum = makedev(0, 0); // GetDevNum();
		_log.Debug("-= After stat =- for device ", major(devnum), ",", minor(devnum));
		struct stat outStat{};
		outStat.st_rdev = devnum;
		PokeData(pid, _stataddr, (void *)&outStat, sizeof(struct stat));
		regs.rax = 0;
	}
	//_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
	//	"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

