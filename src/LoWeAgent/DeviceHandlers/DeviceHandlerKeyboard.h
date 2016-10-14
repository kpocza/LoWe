#pragma once

#include "DeviceHandler.h"
#include <list>
#include "SocketCommunicator.h"

class DeviceHandlerKeyboard : public DeviceHandler
{
	public:
		DeviceHandlerKeyboard(const pid_t pid, const char *path);

		virtual bool IsDeviceAvailable() override;
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
		int _port = 12347;
		SocketCommunicator _socketCommunicator;

		int SendOpcode(char *opCode);
};
