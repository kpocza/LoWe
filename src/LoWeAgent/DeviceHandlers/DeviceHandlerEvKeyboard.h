#pragma once

#include "CommunicatingDeviceHandler.h"
#include <list>
#include "SocketCommunicator.h"
#include <linux/input.h>

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

class DeviceHandlerEvKeyboard : public CommunicatingDeviceHandler
{
	public:
		DeviceHandlerEvKeyboard(const pid_t pid, const string path);

		virtual string GetFixupScript() const override;
		virtual void ExecuteBefore(const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(const long syscall, user_regs_struct &regs) override;

	private:
		long _ioctlop;
		long _ioctladdr;
		long _writeaddr;
		long _writelen;
		long _readaddr;
		long _readlen;

		bool _isEnabled;
		long _lastMillisec;
		
		long relbits[NBITS(REL_MAX + 1)]={0};
		long absbits[NBITS(ABS_MAX + 1)]={0};
		long ledbits[NBITS(LED_MAX + 1)]={0};
		long keybits[NBITS(KEY_MAX + 1)]={0};
		long swbits[NBITS(SW_MAX + 1)]={0};
		long mscbits[NBITS(MSC_MAX + 1)]={0};
		long ffbits[NBITS(FF_MAX + 1)]={0};
		long sndbits[NBITS(SND_MAX + 1)]={0};
		int keystate[96/sizeof(int)]={0};
		long ledstate=0;
		long swstate=0;
		long sndstate=0;
	   	unsigned long ev[NBITS(EV_MAX)];
};
