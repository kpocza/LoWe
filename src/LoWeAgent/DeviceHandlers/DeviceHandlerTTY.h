#pragma once

#include "DeviceHandler.h"
#include <linux/vt.h>
#include <linux/kd.h>

class DeviceHandlerTTY : public DeviceHandler
{
	public:
		DeviceHandlerTTY(const pid_t pid, const string path);

		virtual bool IsDeviceAvailable() override;
		virtual string GetFixupScript() const override;
		virtual void ExecuteBefore(const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(const long syscall, user_regs_struct &regs) override;

	private:
		long _ioctlop;
		long _ioctladdr;

		vt_stat _vt_stat;
		vt_mode _vt_mode;
		int _kdmode;
		int _kbmode;
};
