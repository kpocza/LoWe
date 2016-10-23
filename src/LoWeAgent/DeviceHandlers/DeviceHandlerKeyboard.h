#pragma once

#include "CommunicatingDeviceHandler.h"
#include <list>
#include "SocketCommunicator.h"

class DeviceHandlerKeyboard : public CommunicatingDeviceHandler
{
	public:
		DeviceHandlerKeyboard(const pid_t pid, const string path);

		virtual string GetFixupScript() const override;
		virtual void ExecuteBefore(const long syscall, user_regs_struct &regs) override;
		virtual void ExecuteAfter(const long syscall, user_regs_struct &regs) override;
		virtual void SetPort(int port) override;
		virtual int GetPort() const override;

	private:
		long _ioctlop;
		long _ioctladdr;
		long _writeaddr;
		long _writelen;
		long _readaddr;
		long _readlen;

		bool _isEnabled;
		long _lastMillisec;
		static int _keybPort;
};
