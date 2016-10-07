#pragma once

#include "DeviceHandler.h"
#include <list>

class DeviceHandlerMice : public DeviceHandler
{
	public:
		DeviceHandlerMice(const pid_t pid, const char *path);

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
		int _dataIdx = 0;

		std::list<char> _resp;
};
