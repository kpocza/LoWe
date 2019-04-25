#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerSysDirectory.h"
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>

DeviceHandlerSysDirectory::DeviceHandlerSysDirectory(const pid_t pid, const string openpath): 
	DeviceHandler(pid, openpath, "sysdir", "")
{
	directoryMap.emplace("char", "/tmp/char");
	directoryMap.emplace("tmp", "/tmp");

	directoryMap.emplace("/run/udev/data/c13:1", "/tmp/udevmice");
	directoryMap.emplace("/sys/dev//tmp/char/13:1/uevent", "/tmp/mice_uevent");

	directoryMap.emplace("/run/udev/data/c13:2", "/tmp/udevkbd");
	directoryMap.emplace("/sys/dev//tmp/char/13:2/uevent", "/tmp/kbd_uevent");
}

bool DeviceHandlerSysDirectory::IsDeviceAvailable()
{
	return true;
}

string DeviceHandlerSysDirectory::GetFixupScript() const
{
	return "";
}

void DeviceHandlerSysDirectory::ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;
	if(syscall == SYS_open || syscall == SYS_openat)
	{
		uint64_t pathAddr = 0;

		if(syscall == SYS_open) {
			pathAddr = regs.rdi;
		} else {
			pathAddr = regs.rsi;
		}

		_log.Info("-= Before open/at =-");

		auto mapIt = directoryMap.find(_openpath);
		if(mapIt != directoryMap.end()) {
			PokeData(pid, pathAddr, mapIt->second.c_str(), mapIt->second.size() + 1);
			_log.Info(mapIt->first, " -> ", mapIt->second);
		}
	}
	if(syscall == SYS_lstat) {
		_log.Info("-= Before lstat =-");
		regs.orig_rax = -1;
	}
	if(syscall == SYS_readlinkat) {
		_log.Info("-= Before readlinkat  =-");
		if(_openpath == "/sys/dev//tmp/char/13:1/subsystem")
		{
			const char* replacement = "/tmp/char/13:1/subsystem";
			PokeData(pid, regs.rsi, replacement, strlen(replacement) + 1);
			_log.Info("/sys/dev//tmp/char/13:1/subsystem -> /tmp/char/13:1/subsystem");
		}
		if(_openpath == "/sys/dev//tmp/char/13:2/subsystem")
		{
			const char* replacement = "/tmp/char/13:2/subsystem";
			PokeData(pid, regs.rsi, replacement, strlen(replacement) + 1);
			_log.Info("/sys/dev//tmp/char/13:2/subsystem -> /tmp/char/13:2/subsystem");
		}		
	}

	_log.Info("Syscall:", syscall);
	_log.Debug("regs. rax:", (long)regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

void DeviceHandlerSysDirectory::ExecuteAfter(pid_t pid, const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	if(syscall == SYS_lstat) {
		_log.Info("-= After lstat =-");
		if(_openpath == "/sys/dev//tmp/char/13:1")
		{
			struct stat outStat{};
			memset(&outStat, 0xff, sizeof(struct stat));
			outStat.st_rdev = makedev(13, 1);
			PokeData(pid, _stataddr, (void *)&outStat, sizeof(struct stat));
			regs.rax = 0;
		}
		if(_openpath == "/sys/dev//tmp/char/13:2")
		{
			struct stat outStat{};
			memset(&outStat, 0xff, sizeof(struct stat));
			outStat.st_rdev = makedev(13, 2);
			PokeData(pid, _stataddr, (void *)&outStat, sizeof(struct stat));
			regs.rax = 0;
		}	
	}

	_log.Debug("regs. rax:", (long)regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

