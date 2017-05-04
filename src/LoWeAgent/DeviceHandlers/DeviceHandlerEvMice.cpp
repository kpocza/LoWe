#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "DeviceHandlerEvMice.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <asm-generic/ioctls.h>

DeviceHandlerEvMice::DeviceHandlerEvMice(const pid_t pid, const string openpath): 
	EvdevDeviceHandler(pid, openpath, "mice", "MICE")
{
	_lastMillisec = 0;
	_lastLeftButton = false;
	_lastRightButton = false;
	_lastX = 1280/2;
	_lastY = 720/2;
}

long DeviceHandlerEvMice::GetEv() const
{
	return (1LL << EV_REL) + (1LL<<EV_ABS) + (1LL<<EV_KEY);
}

void DeviceHandlerEvMice::SetRelBits()
{
	relbits[REL_WHEEL/BITS_PER_LONG]|=(1LL << (REL_WHEEL % BITS_PER_LONG));
}

void DeviceHandlerEvMice::SetAbsBits()
{
	absbits[ABS_X/BITS_PER_LONG]|=(1LL << (ABS_X % BITS_PER_LONG));
	absbits[ABS_Y/BITS_PER_LONG]|=(1LL << (ABS_Y % BITS_PER_LONG));
}

void DeviceHandlerEvMice::SetKeyBits()
{
	keybits[BTN_LEFT/BITS_PER_LONG]|=(1LL << (BTN_LEFT % BITS_PER_LONG));
	keybits[BTN_RIGHT/BITS_PER_LONG]|=(1LL << (BTN_RIGHT % BITS_PER_LONG));
	keybits[BTN_WHEEL/BITS_PER_LONG]|=(1LL << (BTN_WHEEL % BITS_PER_LONG));
}

string DeviceHandlerEvMice::GetName() const
{
	return "mice";
}

void DeviceHandlerEvMice::PreEnabling()
{
	_log.Info("Enable mouse");
}

void DeviceHandlerEvMice::ReadLogic(user_regs_struct &regs)
{
	int size = 0;

	if(_isEnabled)
	{
		long now = GetTimeMillisec();
		if(now - _lastMillisec > 100)
		{
			_lastMillisec = now;
			SendOpcode("READ");
			char resp[1+2*4+1];
			_socketCommunicator.Recv((char *)&resp, 10);

			bool leftButtonDown = resp[0]&1;
			bool rightButtonDown = resp[0]&2;

			int xabs = *(int *)&resp[1];
			int yabs = *(int *)&resp[5];

			int wheel = (int)resp[9];

			int cnt = 0;

			timeval t;
			gettimeofday(&t, NULL);
			if(_lastLeftButton!= leftButtonDown)
			{
				_events[cnt].time = t;
				_events[cnt].type = EV_KEY;
				_events[cnt].code = BTN_LEFT;
				_events[cnt].value = leftButtonDown ? 1 : 0;
				cnt++;
				_lastLeftButton = leftButtonDown;
			}

			if(_lastRightButton!= rightButtonDown)
			{
				_events[cnt].time = t;
				_events[cnt].type = EV_KEY;
				_events[cnt].code = BTN_RIGHT;
				_events[cnt].value = rightButtonDown ? 1 : 0;
				cnt++;
				_lastRightButton = rightButtonDown;
			}

			if(_lastX!= xabs)
			{
				_events[cnt].time = t;
				_events[cnt].type = EV_ABS;
				_events[cnt].code = ABS_X;
				_events[cnt].value = xabs;
				cnt++;
				_lastX = xabs;
			}

			if(_lastY!= yabs)
			{
				_events[cnt].time = t;
				_events[cnt].type = EV_ABS;
				_events[cnt].code = ABS_Y;
				_events[cnt].value = yabs;
				cnt++;
				_lastY = yabs;
			}
				
			if(wheel!= 0)
			{
				_events[cnt].time = t;
				_events[cnt].type = EV_REL;
				_events[cnt].code = REL_WHEEL;
				_events[cnt].value = wheel;
				cnt++;
			}

			if(cnt > 0)
			{
				_events[cnt].time = t;
				_events[cnt].type = EV_SYN;
				_events[cnt].code = SYN_REPORT;
				_events[cnt].value = 0;
				cnt++;
				size = cnt * sizeof(input_event);
				PokeData(_readaddr, _events, size);
			}
		}
		else
		{
			usleep(1000);
		}
	}
	else
	{
		if(size > 0)
		{
			char data[size];
			memset(data, 0, size);
			PokeData(_readaddr, data, size);
		}
	}

	regs.rax = size;
	ptrace(PTRACE_SETREGS, _pid, NULL, &regs);

	_log.Debug("Read size:", size);
}

