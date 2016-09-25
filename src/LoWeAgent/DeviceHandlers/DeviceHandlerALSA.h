#pragma once

#include "DeviceHandler.h"
#include <sys/ioctl.h>
#include <sound/asound.h>
#include "SocketCommunicator.h"

class DeviceHandlerALSA : public DeviceHandler
{
	public:
		DeviceHandlerALSA(const pid_t pid, const char *path);

		virtual bool IsDeviceAvailable() override;
		virtual string GetFixupScript() const override;
		virtual void ExecuteBefore(const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(const long syscall, user_regs_struct &regs) override;
	private:
		int SendOpcode(char *opcode);
		void ExecuteBeforeControl(const long syscall, user_regs_struct &regs);
		void ExecuteAfterControl(const long syscall, user_regs_struct &regs);
		void ExecuteBeforePCM(const long syscall, user_regs_struct &regs);
		void ExecuteAfterPCM(const long syscall, user_regs_struct &regs);

		enum ALSADevType {
			Undefined = 0,
			Control = 1,
			Load = 2,
			PCM = 3
		};

		int _port = 12345;

		long _ioctlop;
		long _ioctladdr;
		ALSADevType _devType;
		snd_ctl_card_info _card_info;
		snd_pcm_info _pcm_info;
		snd_pcm_sync_ptr _pcm_sync_ptr;
		snd_pcm_hw_params _snd_pcm_hw_params;
		snd_pcm_status _snd_pcm_status;

		SocketCommunicator _socketCommunicator;
};
