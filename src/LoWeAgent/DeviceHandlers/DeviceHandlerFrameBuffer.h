#pragma once

#include "DeviceHandler.h"
#include <linux/fb.h>

class DeviceHandlerFrameBuffer : public DeviceHandler
{
	public:
		DeviceHandlerFrameBuffer(const pid_t pid, const string path);

		virtual void ExecuteBefore(const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(const long syscall, user_regs_struct &regs) override;

	private:
		struct fb_var_screeninfo _fb_vinfo, _fb_vinfonew;
		struct fb_fix_screeninfo _fb_finfo;
		struct fb_con2fbmap _fb_con2fbmap;
		long _ioctlop;
		long _ioctladdr;
};
