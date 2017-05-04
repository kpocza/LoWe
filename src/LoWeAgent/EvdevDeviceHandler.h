#pragma once

#include "CommunicatingDeviceHandler.h"
#include <list>
#include "SocketCommunicator.h"
#include <linux/input.h>

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

class EvdevDeviceHandler : public CommunicatingDeviceHandler
{
	public:
		EvdevDeviceHandler(const pid_t pid, const string path, const string logName, const string exposerId);

		virtual string GetFixupScript() const override;
		virtual void ExecuteBefore(const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(const long syscall, user_regs_struct &regs) override;

	protected:
		virtual long GetEv() const=0;
		virtual void SetRelBits()=0;
		virtual void SetAbsBits()=0;
		virtual void SetKeyBits()=0;
		virtual string GetName() const=0;
		virtual void PreEnabling()=0;
		virtual void ReadLogic(user_regs_struct &regs)=0;

	protected:
		bool _isEnabled;
		long _ioctlop;
		long _ioctladdr;
		long _writeaddr;
		long _writelen;
		long _readaddr;
		long _readlen;

		struct input_event _events[100];
	
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
