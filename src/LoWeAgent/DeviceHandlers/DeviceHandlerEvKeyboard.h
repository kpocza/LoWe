#pragma once

#include <EvdevDeviceHandler.h>

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
		virtual void ReadLogic(user_regs_struct &regs) override;
	
	private:
		long _lastMillisec;
};
