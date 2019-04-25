#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerFrameBuffer.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

FrameBufferTransporter *DeviceHandlerFrameBuffer::_frameBufferTransporter = new FrameBufferTransporter();

DeviceHandlerFrameBuffer::DeviceHandlerFrameBuffer(const pid_t pid, const string openpath): 
	CommunicatingDeviceHandler(pid, openpath, "fb", "FBUF")
{
	int resx = 1280,resy=720,pixsize=4;

        strcpy(_fb_finfo.id, "LXSSEMU");    /* identification string eg "TT Builtin" */
        _fb_finfo.smem_start=0;       /* Start of frame buffer mem */
        _fb_finfo.smem_len=resx*resy*pixsize;                 /* Length of frame buffer mem */
        _fb_finfo.type=FB_TYPE_PACKED_PIXELS;                     /* see FB_TYPE_*                */
        _fb_finfo.type_aux=0;                 /* Interleave for interleaved Planes */
        _fb_finfo.visual=FB_VISUAL_TRUECOLOR;                   /* see FB_VISUAL_*              */
        _fb_finfo.xpanstep=0;                 /* zero if no hardware panning  */
        _fb_finfo.ypanstep=0;                 /* zero if no hardware panning  */
        _fb_finfo.ywrapstep=0;                /* zero if no hardware ywrap    */
        _fb_finfo.line_length=resx*pixsize;              /* length of a line in bytes    */
        _fb_finfo.mmio_start=0;       /* Start of Memory Mapped I/O   (physical address) */
        _fb_finfo.mmio_len=resx*resy*pixsize;                 /* Length of Memory Mapped I/O  */
        _fb_finfo.accel=FB_ACCEL_NONE;                    /* Indicate to driver which   specific chip/card we have  */
        _fb_finfo.capabilities=0;             /* see FB_CAP_*                 */

	_fb_vinfo.xres=resx;                     /* visible resolution           */
        _fb_vinfo.yres=resy;
        _fb_vinfo.xres_virtual=resx;             /* virtual resolution           */
        _fb_vinfo.yres_virtual=resy;
        _fb_vinfo.xoffset=0;                  /* offset from virtual to visible */
        _fb_vinfo.yoffset=0;                  /* resolution                   */
        _fb_vinfo.bits_per_pixel=32;           /* guess what                   */
        _fb_vinfo.grayscale=0;                /* 0 = color, 1 = grayscale,    */
        _fb_vinfo.red.offset=16;         /* bitfield in fb mem if true color, */
        _fb_vinfo.red.length=8;         /* bitfield in fb mem if true color, */
        _fb_vinfo.green.offset=8;       /* else only length is significant */
        _fb_vinfo.green.length=8;       /* else only length is significant */
        _fb_vinfo.blue.offset=0;
        _fb_vinfo.blue.length=8;
        _fb_vinfo.transp.offset=24;      /* transparency                 */
        _fb_vinfo.transp.length=8;      /* transparency                 */
        _fb_vinfo.nonstd=0;                   /* != 0 Non standard pixel format */
        _fb_vinfo.activate=FB_ACTIVATE_NOW;                 /* see FB_ACTIVATE_*            */
        _fb_vinfo.height=450;                   /* height of picture in mm    */
        _fb_vinfo.width=800;                    /* width of picture in mm     */
        _fb_vinfo.accel_flags=0;              /* (OBSOLETE) see fb_info.flags */
        _fb_vinfo.pixclock=0;                 /* pixel clock in ps (pico seconds) */
        _fb_vinfo.left_margin=0;              /* time from sync to picture    */
        _fb_vinfo.right_margin=0;             /* time from picture to sync    */
        _fb_vinfo.upper_margin=0;             /* time from sync to picture    */
        _fb_vinfo.lower_margin=0;
        _fb_vinfo.hsync_len=0;                /* length of horizontal sync    */
        _fb_vinfo.vsync_len=0;                /* length of vertical sync      */
        _fb_vinfo.sync=0;                     /* see FB_SYNC_*                */
        _fb_vinfo.vmode=FB_VMODE_NONINTERLACED;                    /* see FB_VMODE_*               */
        _fb_vinfo.rotate=0;                   /* angle we rotate counter clockwise */
        _fb_vinfo.colorspace=0;               /* colorspace for FOURCC-based modes */

	_fb_con2fbmap.console = 0;
	_fb_con2fbmap.framebuffer = 0;


	_log.Info("Path:", _openpath);
}

void DeviceHandlerFrameBuffer::ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;
	if(syscall == SYS_open)
	{
		_log.Info("-= Before open =-");
		if(_openpath == "/dev/fb0")
		{
			char fb0[8];
			PeekData(pid, regs.rdi, fb0, 8);
			// /dev/tty0 -> /dev/tty
			fb0[1] = 't';
			fb0[2] = 'm';
			fb0[3] = 'p';
			PokeData(pid, regs.rdi, fb0, 8);
			_log.Info("/dev/fb0 -> /tmp/fb0");
		}	
	}
	if(syscall == SYS_openat)
	{
		_log.Info("-= Before open =-");
		if(_openpath == "/dev/fb0")
		{
			char fb0[8];
			PeekData(pid, regs.rsi, fb0, 8);
			// /dev/tty0 -> /dev/tty
			fb0[1] = 't';
			fb0[2] = 'm';
			fb0[3] = 'p';
			PokeData(pid, regs.rsi, fb0, 8);
			_log.Info("/dev/fb0 -> /tmp/fb0");
		}	
	}
	if(syscall == SYS_ioctl)
	{
		_log.Info("-= Before ioctl =-");
		_log.Debug("Ioctl regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		_ioctlop = regs.rsi;
		_ioctladdr = regs.rdx;
		if(_ioctlop == FBIOGET_FSCREENINFO)
		{
			_log.Info("FBIOGET_FSCREENINFO");	
		}
		else if(_ioctlop == FBIOGET_VSCREENINFO)
		{
			_log.Info("FBIOGET_VSCREENINFO");	
		}
		else if(_ioctlop == FBIOPUT_VSCREENINFO)
		{
			_log.Info("FBIOPUT_VSCREENINFO");	
		}
		else if(_ioctlop == FBIOPUTCMAP)
		{
			_log.Info("FBIOPUTCMAP");	
		}
		else if(_ioctlop == FBIOBLANK)
		{
			_log.Info("FBIOBLANK");	
		}
		else if(_ioctlop == FBIOPAN_DISPLAY)
		{
			_log.Info("FBIOPAN_DISPLAY");	
		}
		else if(_ioctlop == FBIOGET_CON2FBMAP)
		{
			_log.Info("FBIOGET_CON2FBMAP");
		}
		else if(_ioctlop == FBIOPUT_CON2FBMAP)
		{
			_log.Info("FBIOPUT_CON2FBMAP");
		}
		else
		{
			_log.Error("unknown ioctl op ", _ioctlop);
		}
		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else if(syscall == SYS_mmap)
	{
		_log.Info("MMAP");
// waiting for the fix: https://github.com/Microsoft/BashOnWindows/issues/1491
//		regs.r10 = MAP_PRIVATE|MAP_ANONYMOUS;
//		regs.r8 = -1;
//		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
		_frameBufferTransporter->StopThreads();
	}
	else
	{
		_log.Info("Other syscall:", syscall);
	}
}

void DeviceHandlerFrameBuffer::ExecuteAfter(pid_t pid, const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	if(_syscallbefore == SYS_open || _syscallbefore == SYS_openat)
	{
		_log.Info("-= After open =-");
		int len = _fb_vinfo.xres * _fb_vinfo.yres * _fb_vinfo.bits_per_pixel/8;
		std::string realpath = _openpath;
		realpath.replace(0, 4, "/tmp");
		_log.Info("Trunace path:", realpath);
		int trunres = truncate(realpath.c_str(), len);
		_log.Info("Truncate result:", trunres);
	}
	else if(_syscallbefore == SYS_ioctl) 
	{
		_log.Info("-= After ioctl =-");
		if(_ioctlop == FBIOGET_FSCREENINFO) {
			_log.Info("FBIOGET_FSCREENINFO");
			regs.rax = 0;
			PokeData(pid, _ioctladdr, &_fb_finfo, sizeof(_fb_finfo));
		}
		if(_ioctlop == FBIOGET_VSCREENINFO) {
			_log.Info("FBIOGET_VSCREENINFO");
			regs.rax = 0;
			PokeData(pid, _ioctladdr, &_fb_vinfo, sizeof(_fb_vinfo));
		}
		if(_ioctlop == FBIOPUT_VSCREENINFO) {
			_log.Info("FBIOPUT_VSCREENINFO");
			regs.rax = 0;
			PeekData(pid, _ioctladdr, &_fb_vinfonew, sizeof(_fb_vinfo));
			_log.Info("xres:", _fb_vinfonew.xres, "yres:", _fb_vinfonew.yres,
				"xresv:", _fb_vinfonew.xres_virtual, "yresv:", _fb_vinfonew.yres_virtual,
				"bpp:", _fb_vinfonew.bits_per_pixel,
				"width:", _fb_vinfonew.width, "height:", _fb_vinfonew.height);
		}
		if(_ioctlop == FBIOPUTCMAP) {
			_log.Info("FBIOPUTCMAP");
			regs.rax = 0;
		}
		if(_ioctlop == FBIOBLANK) {
			_log.Info("FBIOBLANK");
			regs.rax = 0;
		}
		if(_ioctlop == FBIOPAN_DISPLAY) {
			_log.Info("FBIOPAN_DISPLAY");
			regs.rax = 0;
		}
		if(_ioctlop == FBIOGET_CON2FBMAP) {
			_log.Info("FBIOGET_CON2FBMAP");
			regs.rax = 0;
			PokeData(pid, _ioctladdr, (char *)&_fb_con2fbmap, sizeof(_fb_con2fbmap));
		}
		if(_ioctlop == FBIOPUT_CON2FBMAP) {
			_log.Info("FBIOPUT_CON2FBMAP");
			regs.rax = 0;
		}
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else if(_syscallbefore == SYS_mmap)
	{
		_log.Debug("After MMAP:", regs.rax);
		if(_socketCommunicator.Open("127.0.0.1", GetPort()))
		{
			_log.Info("socket opened");
			SendOpcode("INIT");
			int params[3];
			params[0] = _fb_vinfo.xres;
			params[1] = _fb_vinfo.yres;
			params[2] = _fb_vinfo.bits_per_pixel;
			_socketCommunicator.Send((char *)params, sizeof(params));
			long address = regs.rax;
			_frameBufferTransporter->StartThreads(pid, _fb_finfo.mmio_len, address, &_socketCommunicator);
		}
		else 
		{
			_log.Error("socket open failed");
			regs.rax = -1;
			ptrace(PTRACE_SETREGS, pid, NULL, &regs);
		}
	}

	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

