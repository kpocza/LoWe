#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerCatchAll.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

DeviceHandlerCatchAll::DeviceHandlerCatchAll(const pid_t pid, const char *openpath): 
	DeviceHandler(pid, openpath, Log("catchall"))
{

}

bool DeviceHandlerCatchAll::IsDeviceAvailable()
{
	return true;
}

string DeviceHandlerCatchAll::GetFixupScript() const
{
	return "";
}

void DeviceHandlerCatchAll::ExecuteBefore(const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;
	_log.Info("Syscall:", syscall);
	_log.Debug("regs. rax:", (long)regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

void DeviceHandlerCatchAll::ExecuteAfter(const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	_log.Debug("regs. rax:", (long)regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

