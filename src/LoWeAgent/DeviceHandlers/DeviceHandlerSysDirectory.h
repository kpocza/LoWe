#pragma once

#include "DeviceHandler.h"

#include <unordered_map>

class DeviceHandlerSysDirectory : public DeviceHandler
{
	public:
		DeviceHandlerSysDirectory(const pid_t pid, const string path);

		virtual bool IsDeviceAvailable() override;
		virtual string GetFixupScript() const override;
		virtual void ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(pid_t pid, const long syscall, user_regs_struct &regs) override;

	private:
		long _ioctlop;
		long _ioctladdr;
		long _stataddr;

		unordered_map<string, string> directoryMap;

};
