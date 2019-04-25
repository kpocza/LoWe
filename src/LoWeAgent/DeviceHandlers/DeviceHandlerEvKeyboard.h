#pragma once

#include <EvdevDeviceHandler.h>
#include <sys/sysmacros.h>

class DeviceHandlerEvKeyboard : public EvdevDeviceHandler
{
	public:
		DeviceHandlerEvKeyboard(const pid_t pid, const string path);

	protected:
		virtual long GetEv() const override;
		virtual void SetRelBits() override;
		virtual void SetAbsBits() override;
		virtual void SetKeyBits() override;
		virtual string GetName() const override;
		virtual void PreEnabling() override;
		virtual void ReadLogic(pid_t pid, user_regs_struct &regs) override;
		virtual dev_t GetDevNum() override {
			// Input, minor 2
			return makedev(13, 2);
		}
	
	private:
		long _lastMillisec;
};
