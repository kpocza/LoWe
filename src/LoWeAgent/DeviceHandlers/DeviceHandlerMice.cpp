#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerMice.h"
#include <iostream>
#include <unistd.h>
#include <asm-generic/ioctls.h>

// http://www.win.tue.nl/~aeb/linux/kbd/scancodes-13.html

#define MICE_RESET 0xff
#define MICE_SETRATE 0xf3
#define MICE_GETMOUSEID 0xf2
#define MICE_SETDEFAULTS 0xf6
#define MICE_SETSCALE1 0xe6
#define MICE_SETRESOLUTION 0xe8
#define MICE_SETSTREAMMODE 0xea
#define MICE_ENABLE 0xf4
#define MICE_ACK 0xfa
#define MICE_NACK 0xfe
#define MICE_READY 0xaa
#define MICE_ID_MOUSE 0x0
#define MICE_MOUSEID_EXPLORER 0x4

DeviceHandlerMice::DeviceHandlerMice(const pid_t pid, const string openpath): 
	CommunicatingDeviceHandler(pid, openpath, "mice", "MICE")
{
	_log.Info("Path:", _openpath);
	_willBeEnabled = false;
	_isEnabled = false;
	_isStreamMode = false;
	_rate = 0;
	_dataIdx = 0;
	_resolution = 0;
	_curCommand = -1;
	_cmdAcked = false;
	_lastMillisec = 0;
}

string DeviceHandlerMice::GetFixupScript() const
{
	if(!HasPermissions())
		return GetFixupScriptCore();

	return "";
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
		_writeaddr = regs.rsi;
		_writelen = regs.rdx;
		char data[8];
		PeekData(_writeaddr, &data, _writelen);

		for(int i =0;i < _writelen;i++)
			_req.push_back((unsigned char)data[i]);

		if(_curCommand == -1)
		{
			_curCommand = _req.front();
			_req.pop_front();
			_resp.clear();
			_cmdAcked = false;
		}

		_log.Info("Current Command:", _curCommand);
		if(_curCommand == MICE_RESET)
		{
			_log.Info("Reset");
			_cmdAcked = true;
			_resp.push_back(MICE_ACK);
			_resp.push_back(MICE_READY);
			_resp.push_back(MICE_ID_MOUSE);
			_curCommand = -1;
		}
		if(_curCommand == MICE_SETDEFAULTS)
		{
			_log.Info("Set defaults");
			_cmdAcked = true;
			_resp.push_back(MICE_ACK);
			_rate = 100;
			_resolution = 4;
			_willBeEnabled = false;
			_isEnabled = false;
			_curCommand = -1;
		}
		else if(_curCommand == MICE_SETRATE)
		{
			_log.Info("Set Rate");
			if(!_cmdAcked)
			{
				_cmdAcked = true;
		 		_resp.push_back(MICE_ACK);
			}

			if(_req.size() > 0)
			{
				_rate = (int)(unsigned char)_req.front();
				_req.pop_front();
				_log.Info("rate:", _rate);
			 	_resp.push_back(MICE_ACK);
				_curCommand = -1;
			}
		}
		else if(_curCommand == MICE_SETRESOLUTION)
		{
			_log.Info("Set Resolution");
			if(!_cmdAcked)
			{
				_cmdAcked = true;
		 		_resp.push_back(MICE_ACK);
			}

			if(_req.size() > 0)
			{
				_resolution = (int)(unsigned char)_req.front();
				_req.pop_front();
				_log.Info("resolution:", _resolution);
			 	_resp.push_back(MICE_ACK);
				_curCommand = -1;
			}
		}
		else if(_curCommand == MICE_GETMOUSEID)
		{
			_log.Info("Get Mouse Id");
			_cmdAcked = true;
			_resp.push_back(MICE_MOUSEID_EXPLORER);
			_curCommand = -1;
		}
		else if(_curCommand == MICE_SETSCALE1)
		{
			_log.Info("Set Scale1");
			_cmdAcked = true;
			_resp.push_back(MICE_ACK);
			_curCommand = -1;
		}
		else if(_curCommand == MICE_SETSTREAMMODE)
		{
			_log.Info("Set steam mode");
			_cmdAcked = true;
			_isStreamMode = true;
			_resp.push_back(MICE_ACK);
			_curCommand = -1;
		}
		else if(_curCommand == MICE_ENABLE)
		{
			_log.Info("Enable mouse");
			_willBeEnabled = true;
			_cmdAcked = true;
			_isEnabled = false;
			if(_socketCommunicator.Open("127.0.0.1", GetPort()))
			{
				_log.Info("socket opened");
				SendOpcode("INIT");
				_resp.push_back(MICE_ACK);
			}
			else 
			{
				_log.Error("socket open failed");
				_resp.push_back(MICE_NACK);
			}
			_curCommand = -1;
		}
		else if(_curCommand == 0)
		{
			_log.Info("0");
			_cmdAcked = true;
			_resp.push_back(MICE_ACK);
			_curCommand = -1;
		}
		else
		{
			_log.Info("Unknown mouse control code:", _curCommand);
			_curCommand = -1;
		}

		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
	}
	else if(syscall == SYS_read)
	{
		if(!_isEnabled)
			_log.Info("-= Before read =-");
		else
			_log.Debug("-= Before read =-");
		_log.Debug("Read regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);

		_readaddr = regs.rsi;
		_readlen = regs.rdx;

		if(!_isEnabled)
			_log.Info("Read size:", _readlen);
		else
			_log.Debug("Read size:", _readlen);

		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);
		if(_isEnabled)
		{
			long now = GetTimeMillisec();
			if(now - _lastMillisec > 100 || _dataIdx!= 0)
			{
				if(_dataIdx == 0)
				{
					_lastMillisec = now;
					_resp.clear();
					SendOpcode("READ");
					char resp[1+2*4+1];
					_socketCommunicator.Recv((char *)&resp, 10);

					if(resp[0] != (char)0xff)
					{
						bool leftButtonDown = resp[0]&1;
						bool rightButtonDown = resp[0]&2;

						int xdiff = *(int *)&resp[1];
						int ydiff = *(int *)&resp[5];

						char b1 = 8;
						if(leftButtonDown)
							b1++;
						if(rightButtonDown)
							b1+=2;

						char b2 = 0;	
						char b3 = 0;
						if(xdiff >= 0)
						{
							b2 = (char)xdiff;
						}
						else
						{
							b1+= 0x10;
							b2 = (char)xdiff;
						}
						if(ydiff <= 0)
						{
							b3 = (char)-ydiff;
						}
						else
						{
							b1+= 0x20;
							b3 = 256-(char)ydiff;
						}
						int wheel = (int)resp[9];

						char b4 = 0;
						if(wheel == 1) b4 = 1;
						if(wheel == -1) b4 = 0xf;

						_resp.push_back(b1);
						_resp.push_back(b2);
						_resp.push_back(b3);
						_resp.push_back(b4);
					}
				}

				_dataIdx+= _readlen;
				if(_dataIdx >= 4)
				{
					_dataIdx=0;
				}
			}
			else
			{
				usleep(1000);
			}
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
			_req.clear();
			_curCommand = -1;
			_willBeEnabled = false;
			_cmdAcked = false;
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
		if(!_isEnabled)
			_log.Info("-= After read =-");
		else
			_log.Debug("-= After read =-");

		unsigned char response[_readlen];
		int size = 0;
		while(!_resp.empty() && size < _readlen)
		{
			unsigned char r = _resp.front(); 
			response[size] = r;

			if(!_isEnabled)
				_log.Info("Resp:", (int)r);
			else
				_log.Debug("Resp:", (int)r);
			
			_resp.pop_front();
			size++;
		}
		PokeData(_readaddr, response, size);
		regs.rax = size;
		ptrace(PTRACE_SETREGS, _pid, NULL, &regs);

		if(!_isEnabled)
			_log.Info("Read size:", size);
		else
			_log.Debug("Read size:", size);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

