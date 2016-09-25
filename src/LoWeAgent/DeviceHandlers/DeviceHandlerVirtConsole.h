#pragma once

#include "DeviceHandler.h"

class DeviceHandlerVirtConsole : public DeviceHandler
{
	public:
		DeviceHandlerVirtConsole(const pid_t pid, const char *path);

		virtual void ExecuteBefore(const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(const long syscall, user_regs_struct &regs) override;

	private:
		long _ioctlop;
		long _ioctladdr;
};
