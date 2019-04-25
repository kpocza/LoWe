#pragma once

#include "CommunicatingDeviceHandler.h"
#include "FrameBufferTransporter.h"
#include <linux/fb.h>

class DeviceHandlerFrameBuffer : public CommunicatingDeviceHandler
{
	public:
		DeviceHandlerFrameBuffer(const pid_t pid, const string path);

		virtual void ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(pid_t pid, const long syscall, user_regs_struct &regs) override;

	private:
		struct fb_var_screeninfo _fb_vinfo, _fb_vinfonew;
		struct fb_fix_screeninfo _fb_finfo;
		struct fb_con2fbmap _fb_con2fbmap;
		long _ioctlop;
		long _ioctladdr;

		static FrameBufferTransporter *_frameBufferTransporter;
};

