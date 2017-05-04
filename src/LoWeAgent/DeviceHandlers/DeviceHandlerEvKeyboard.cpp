#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "DeviceHandlerEvKeyboard.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <asm-generic/ioctls.h>

DeviceHandlerEvKeyboard::DeviceHandlerEvKeyboard(const pid_t pid, const string openpath): 
	EvdevDeviceHandler(pid, openpath, "kbd", "KEYB")
{
	_lastMillisec = 0;
}

long DeviceHandlerEvKeyboard::GetEv() const
{
	return (1LL<<EV_KEY);
}

void DeviceHandlerEvKeyboard::SetRelBits()
{
}

void DeviceHandlerEvKeyboard::SetAbsBits()
{
}

void DeviceHandlerEvKeyboard::SetKeyBits()
{
	keybits[0]=(unsigned long)0xffffffffffffffff;
	keybits[1]=(unsigned long)0xffffffffffffffff;
}

string DeviceHandlerEvKeyboard::GetName() const
{
	return "kbd";
}

void DeviceHandlerEvKeyboard::PreEnabling()
{
	_log.Info("Enable keyboard");
}

void DeviceHandlerEvKeyboard::ReadLogic(user_regs_struct &regs)
{
	int size = 0;
	if(_isEnabled)
	{
		long now = GetTimeMillisec();
		if(now - _lastMillisec > 100)
		{
			_lastMillisec = now;
			SendOpcode("READ");

			int maxItems = _readlen/sizeof(_events[0]);
			int maxItemCount = sizeof(_events)/sizeof(_events[0]);
			
			if(maxItemCount - 1 < maxItems)
				maxItems = maxItemCount - 1;

			_socketCommunicator.Send((char *)&maxItems, 4);

			int cnt;
			_socketCommunicator.Recv((char *)&cnt, 4);
			if(cnt > 0)
			{
				char results[cnt];
				_socketCommunicator.Recv((char *)&results, cnt);

				timeval t;
				gettimeofday(&t, NULL);
				for(int i = 0;i < cnt;i++)
				{
					char code = results[i];
					_events[i].time = t;
					_events[i].type = EV_KEY;
					_events[i].code = code&0x7f;
					_events[i].value = code&0x80 ? 1 : 0;
				}
				_events[cnt].time = t;
				_events[cnt].type = EV_SYN;
				_events[cnt].code = SYN_REPORT;
				_events[cnt].value = 0;

				size = sizeof(_events[0]) * (cnt+1);
				PokeData(_readaddr, &_events, size);
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

