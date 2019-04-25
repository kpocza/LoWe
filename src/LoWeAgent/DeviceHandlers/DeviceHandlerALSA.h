#pragma once

#include "CommunicatingDeviceHandler.h"
#include <sys/ioctl.h>
#include <sound/asound.h>

class DeviceHandlerALSA : public CommunicatingDeviceHandler
{
	public:
		DeviceHandlerALSA(const pid_t pid, const string path);

		virtual string GetFixupScript() const override;
		virtual void ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(pid_t pid, const long syscall, user_regs_struct &regs) override;

	private:
		void ExecuteBeforeControl(pid_t pid, const long syscall, user_regs_struct &regs);
		void ExecuteAfterControl(pid_t pid, const long syscall, user_regs_struct &regs);
		void ExecuteBeforePCM(pid_t pid, const long syscall, user_regs_struct &regs);
		void ExecuteAfterPCM(pid_t pid, const long syscall, user_regs_struct &regs);

		enum ALSADevType {
			Undefined = 0,
			Control = 1,
			Load = 2,
			PCM = 3
		};


		long _ioctlop;
		long _ioctladdr;
		ALSADevType _devType;
		snd_ctl_card_info _card_info;
		snd_pcm_info _pcm_info;
		snd_pcm_sync_ptr _pcm_sync_ptr;
		snd_pcm_hw_params _snd_pcm_hw_params;
		snd_pcm_status _snd_pcm_status;
};
