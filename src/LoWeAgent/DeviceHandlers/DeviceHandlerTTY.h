#pragma once

#include "DeviceHandler.h"
#include <linux/vt.h>
#include <linux/kd.h>
#include <termios.h>

class DeviceHandlerTTY : public DeviceHandler
{
	public:
		DeviceHandlerTTY(const pid_t pid, const string path);

		virtual bool IsDeviceAvailable() override;
		virtual string GetFixupScript() const override;
		virtual void ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(pid_t pid, const long syscall, user_regs_struct &regs) override;

	private:
		long _ioctlop;
		long _ioctladdr;

		vt_stat _vt_stat;
		vt_mode _vt_mode;
		termios _termios;
		int _kdmode;
		int _kbmode;
};
