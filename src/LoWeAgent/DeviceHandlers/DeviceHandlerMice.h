#pragma once

#include "CommunicatingDeviceHandler.h"
#include <list>
#include "SocketCommunicator.h"

class DeviceHandlerMice : public CommunicatingDeviceHandler
{
	public:
		DeviceHandlerMice(const pid_t pid, const string path);

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

		bool _willBeEnabled;
		bool _isEnabled;
		bool _isStreamMode;
		int _rate;
		int _resolution;
		int _dataIdx = 0;

		int _curCommand;
		bool _cmdAcked;
		std::list<int> _req;
		std::list<char> _resp;

		long _lastMillisec;
};
