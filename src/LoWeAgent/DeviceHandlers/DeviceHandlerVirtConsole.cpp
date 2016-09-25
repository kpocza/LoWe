#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerVirtConsole.h"
#include <string.h>
#include <unistd.h>

DeviceHandlerVirtConsole::DeviceHandlerVirtConsole(const pid_t pid, const char *openpath): 
	DeviceHandler(pid, openpath, Log("vc"))
{
	_log.Info("Path:", _openpath);
}

void DeviceHandlerVirtConsole::ExecuteBefore(const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;
	if(syscall == SYS_ioctl)
	{
		_log.Info("-= Before ioctl  =-");
		_log.Debug("Ioctl regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		_ioctlop = regs.rsi;
		_ioctladdr = regs.rdx;

		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	} 
	else 
	{
		_log.Info("Other syscall:", syscall);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

void DeviceHandlerVirtConsole::ExecuteAfter(const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	if(_syscallbefore == SYS_ioctl) 
	{
		_log.Info("-= After ioctl  =-");
		long l = 3;
		PokeData(_ioctladdr, (char *)&l, sizeof(l));
		regs.rax = 0;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}

	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

