#pragma once

#include <EvdevDeviceHandler.h>
#include <sys/sysmacros.h>

class DeviceHandlerEvMice : public EvdevDeviceHandler
{
	public:
		DeviceHandlerEvMice(const pid_t pid, const string path);

	protected:
		virtual long GetEv() const override;
		virtual void SetRelBits() override;
		virtual void SetAbsBits() override;
		virtual void SetKeyBits() override;
		virtual string GetName() const override;
		virtual void PreEnabling() override;
		virtual void ReadLogic(pid_t pid, user_regs_struct &regs) override;
		virtual dev_t GetDevNum() override {
			// Input, minor 1
			return makedev(13, 1);
		}
	
	private:
		long _lastMillisec;
		bool _lastLeftButton;
		bool _lastRightButton;
		int _lastX;
		int _lastY;
};
