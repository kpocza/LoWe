#pragma once

#include <sys/user.h>
#include "DeviceHandlerRegistry.h"
#include "DeviceHandlerFactory.h"
#include "DeviceHandler.h"
#include "Log.h"
#include <memory>
#include <unordered_set>
#include <unordered_map>

class DeviceHandlerRegistry;
class ProgRuntimeDispatcher;

class ProgRuntimeHandler 
{
	public:
		ProgRuntimeHandler(pid_t pid, int status, DeviceHandlerFactory &deviceHandlerFactory, std::weak_ptr<ProgRuntimeDispatcher> runtimeDispatcher);

		bool Step();

		bool AcceptHandlesFromOtherProcess(const HandleMap& handleMap);

	private:
		bool SpySyscallEnter();
		bool SpySyscallExit();

		bool HasZero(const unsigned long data);
		bool ReadRemoteText(long addr, char *out, int maxlen);

		std::weak_ptr<ProgRuntimeDispatcher> _runtimeDispatcher;

		const pid_t _pid;
		const int _status;
		const DeviceHandlerFactory &_deviceHandlerFactory;
		const Log _log;

		bool _exiting;
		int _syscall;
		char _openpath[256];
		std::shared_ptr<DeviceHandlerRegistry> _deviceHandlerRegistry;
		std::shared_ptr<DeviceHandler> _currentDeviceHandler;
		unordered_set<int> maskedSysCallsForDebug;

		unordered_map<uint64_t, unordered_set<uint64_t>> epollList;
};

