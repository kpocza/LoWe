#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerMice.h"
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <asm-generic/ioctls.h>

// http://www.win.tue.nl/~aeb/linux/kbd/scancodes-13.html

#define GPM_RESET 0xff
#define GPM_SETRATE 0xf3
#define GPM_SETSCALE1 0xe6
#define GPM_SETSTREAMMODE 0xea
#define GPM_ENABLE 0xf4
#define GPM_ACK 0xfa
#define GPM_READY 0xaa
#define GPM_ID_MOUSE 0x0

DeviceHandlerMice::DeviceHandlerMice(const pid_t pid, const char *openpath): 
	DeviceHandler(pid, openpath, "mice")
{
	_log.Info("Path:", _openpath);
	_willBeEnabled = false;
	_isEnabled = false;
	_isStreamMode = false;
	_rate = 0;
	_dataIdx = 0;
}

void DeviceHandlerMice::ExecuteBefore(const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;
	if(syscall == SYS_ioctl)
	{
		_log.Info("-= Before ioctl =-");
		_log.Debug("Ioctl regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		_ioctlop = regs.rsi;
		_ioctladdr = regs.rdx;
		if(_ioctlop == TCFLSH)
		{
			_log.Info("TCFLSH");
			regs.orig_rax = -1;
		}
		else
		{
			_log.Error("unknown ioctl op ", _ioctlop);
		}
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(syscall == SYS_write)
	{
		_log.Info("-= Before write =-");
		_log.Debug("Write regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		char data[8];
		PeekData(_writeaddr, (char *)&data, 8);
		unsigned char firstChar = (unsigned char)data[0];
		_log.Info("First char:", (int)firstChar);
		if(firstChar == GPM_RESET)
		{
			_log.Info("Reset");
			_resp.clear();
			_resp.push_back(GPM_ACK);
			_resp.push_back(GPM_READY);
			_resp.push_back(GPM_ID_MOUSE);
		}
		else if(firstChar == GPM_SETRATE)
		{
			_log.Info("Set Rate");
			_rate = (int)(unsigned char)data[1];
			_log.Info("rate:", _rate);
			_resp.clear();
		 	_resp.push_back(GPM_ACK);
		 	_resp.push_back(GPM_ACK);
		 	//_resp.push_back(data[1]);
		}
		else if(firstChar == GPM_SETSCALE1)
		{
			_log.Info("Set Scale1");
			_resp.clear();
			_resp.push_back(GPM_ACK);
		}
		else if(firstChar == GPM_SETSTREAMMODE)
		{
			_log.Info("Set steam mode");
			_isStreamMode = true;
			_resp.clear();
			_resp.push_back(GPM_ACK);
		}
		else if(firstChar == GPM_ENABLE)
		{
			_log.Info("Enable mouse");
			_willBeEnabled = true;
			_isEnabled = false;
			_resp.clear();
			_resp.push_back(GPM_ACK);
		}
		else
		{
			_log.Info("Unknown mouse control code:", (int)firstChar);
		}

		_writeaddr = regs.rsi;
		_writelen = regs.rdx;
		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(syscall == SYS_read)
	{
		_log.Info("-= Before read =-");
		_log.Debug("Read regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);

		_readaddr = regs.rsi;
		_readlen = regs.rdx;
		_log.Info("Read size:", _readlen);
		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);

		if(_isEnabled)
		{
			if(_dataIdx == 0)
			{
				_resp.clear();
				_resp.push_back(0x20);
				_resp.push_back(0);
				_resp.push_back(0);
				_resp.push_back(0);
			}

			_dataIdx+= _readlen;
			if(_dataIdx >= 4)
				_dataIdx=0;
		}
		if(_willBeEnabled && _readlen == 1)
		{
			_willBeEnabled = false;
			_isEnabled = true;
		}
	}
	else if(syscall!= SYS_open)
	{
		_log.Info("Other syscall:", syscall);
	}
}

void DeviceHandlerMice::ExecuteAfter(const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	if(_syscallbefore == SYS_ioctl) 
	{
		_log.Info("-= After ioctl =-");
		if(_ioctlop == TCFLSH)
		{
			_log.Info("TCFLSH");
			_resp.clear();
			regs.rax = 0;
		}
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(_syscallbefore == SYS_write)
	{
		_log.Info("-= After write =-");
		regs.rax = _writelen;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
		_log.Info("Write size:", _writelen);
	}
	else if(_syscallbefore == SYS_read)
	{
		_log.Info("-= After read =-");
		unsigned char response[_readlen];
		int size = 0;
		while(!_resp.empty() && size < _readlen)
		{
			unsigned char r = _resp.front(); 
			response[size] = r;
			_log.Info("Resp:", (int)r);
			
			_resp.pop_front();
			size++;
		}
		PokeData(_readaddr, (char *)response, size);
		regs.rax = size;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
		_log.Info("Read size:", size);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

