#pragma once

#include <sys/user.h>
#include "DeviceHandlerRegistry.h"
#include "DeviceHandlerFactory.h"
#include "DeviceHandler.h"
#include "Log.h"

class ProgRuntimeHandler 
{
	public:
		ProgRuntimeHandler(pid_t pid, int status, DeviceHandlerFactory &deviceHandlerFactory);

		bool Step();

	private:
		bool SpySyscallEnter();
		bool SpySyscallExit();

		bool HasZero(const unsigned long data);
		bool ReadRemoteText(long addr, char *out, int maxlen);

		const pid_t _pid;
		const int _status;
		const DeviceHandlerFactory &_deviceHandlerFactory;
		const Log _log;

		bool _exiting;
		int _syscall;
		char _openpath[256];
		DeviceHandlerRegistry _deviceHandlerRegistry;
		DeviceHandler *_currentDeviceHandler;
};

