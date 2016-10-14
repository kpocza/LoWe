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
#define GPM_GETMOUSEID 0xf2
#define GPM_SETDEFAULTS 0xf6
#define GPM_SETSCALE1 0xe6
#define GPM_SETRESOLUTION 0xe8
#define GPM_SETSTREAMMODE 0xea
#define GPM_ENABLE 0xf4
#define GPM_ACK 0xfa
#define GPM_NACK 0xfe
#define GPM_READY 0xaa
#define GPM_ID_MOUSE 0x0
#define GPM_MOUSEID_EXPLORER 0x4

DeviceHandlerMice::DeviceHandlerMice(const pid_t pid, const char *openpath): 
	DeviceHandler(pid, openpath, "mice")
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

bool DeviceHandlerMice::IsDeviceAvailable()
{
	if(!HasPermissions())
		return false;

	if(!_socketCommunicator.Open("127.0.0.1", _port))
	{
		_log.Error("Mice exposer socket cannot be opened.");
		_log.Error("Please ensure that LoWeExposer.exe application is running and listening on port", _port);
		_socketCommunicator.Close();
		return false;
	}
	if(!SendOpcode((char *)"MICE"))
	{
		_log.Error("Mice exposer socket cannot be written");
		_log.Error("Please ensure that LoWeExposer.exe application is running and listening on port", _port);
		_socketCommunicator.Close();
		return false;
	}
	char resp[5]={0};
	if(!_socketCommunicator.Recv((char *)&resp, 4) || strcmp(resp, "ECIM"))
	{
		_log.Error("Mice exposer socket cannot be read or bad result");
		_log.Error("Please ensure that LoWeExposer.exe application is running and listening on port", _port);
		_socketCommunicator.Close();
		return false;
	}
	_socketCommunicator.Close();
	return true;
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
		PeekData(_writeaddr, (char *)&data, _writelen);

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
		if(_curCommand == GPM_RESET)
		{
			_log.Info("Reset");
			_cmdAcked = true;
			_resp.push_back(GPM_ACK);
			_resp.push_back(GPM_READY);
			_resp.push_back(GPM_ID_MOUSE);
			_curCommand = -1;
		}
		if(_curCommand == GPM_SETDEFAULTS)
		{
			_log.Info("Set defaults");
			_cmdAcked = true;
			_resp.push_back(GPM_ACK);
			_rate = 100;
			_resolution = 4;
			_willBeEnabled = false;
			_isEnabled = false;
			_curCommand = -1;
		}
		else if(_curCommand == GPM_SETRATE)
		{
			_log.Info("Set Rate");
			if(!_cmdAcked)
			{
				_cmdAcked = true;
		 		_resp.push_back(GPM_ACK);
			}

			if(_req.size() > 0)
			{
				_rate = (int)(unsigned char)_req.front();
				_req.pop_front();
				_log.Info("rate:", _rate);
			 	_resp.push_back(GPM_ACK);
				_curCommand = -1;
			}
		}
		else if(_curCommand == GPM_SETRESOLUTION)
		{
			_log.Info("Set Resolution");
			if(!_cmdAcked)
			{
				_cmdAcked = true;
		 		_resp.push_back(GPM_ACK);
			}

			if(_req.size() > 0)
			{
				_resolution = (int)(unsigned char)_req.front();
				_req.pop_front();
				_log.Info("resolution:", _resolution);
			 	_resp.push_back(GPM_ACK);
				_curCommand = -1;
			}
		}
		else if(_curCommand == GPM_GETMOUSEID)
		{
			_log.Info("Get Mouse Id");
			_cmdAcked = true;
			_resp.push_back(GPM_MOUSEID_EXPLORER);
			_curCommand = -1;
		}
		else if(_curCommand == GPM_SETSCALE1)
		{
			_log.Info("Set Scale1");
			_cmdAcked = true;
			_resp.push_back(GPM_ACK);
			_curCommand = -1;
		}
		else if(_curCommand == GPM_SETSTREAMMODE)
		{
			_log.Info("Set steam mode");
			_cmdAcked = true;
			_isStreamMode = true;
			_resp.push_back(GPM_ACK);
			_curCommand = -1;
		}
		else if(_curCommand == GPM_ENABLE)
		{
			_log.Info("Enable mouse");
			_willBeEnabled = true;
			_cmdAcked = true;
			_isEnabled = false;
			if(_socketCommunicator.Open("127.0.0.1", _port))
			{
				_log.Info("socket opened");
				SendOpcode((char *)"INIT");
				_resp.push_back(GPM_ACK);
			}
			else 
			{
				_log.Error("socket open failed");
				_resp.push_back(GPM_NACK);
			}
			_curCommand = -1;
		}
		else if(_curCommand == 0)
		{
			_log.Info("0");
			_cmdAcked = true;
			_resp.push_back(GPM_ACK);
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
		_log.Info("-= Before read =-");
		_log.Debug("Read regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);

		_readaddr = regs.rsi;
		_readlen = regs.rdx;
		_log.Info("Read size:", _readlen);
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
					SendOpcode((char *)"READ");
					char resp[1+2*4];
					_socketCommunicator.Recv((char *)&resp, 9);

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

						_resp.push_back(b1);
						_resp.push_back(b2);
						_resp.push_back(b3);
						_resp.push_back(0);
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

int DeviceHandlerMice::SendOpcode(char *opCode)
{
	return _socketCommunicator.Send(opCode, 4);
}

