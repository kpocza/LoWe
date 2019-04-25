#pragma once

#include <sys/user.h>
#include "ProgRuntimeHandler.h"
#include "DeviceHandlerFactory.h"
#include "Log.h"
#include <unordered_map>
#include <memory>

using namespace std;

class ProgRuntimeHandler;
class DeviceHandlerRegistry;

class ProgRuntimeDispatcher : public std::enable_shared_from_this<ProgRuntimeDispatcher>
{
	public:
		ProgRuntimeDispatcher(DeviceHandlerFactory &deviceHandlerFactory);
		bool Init(pid_t pid, bool isExec);
		bool Spin();

		bool CloneHandlesToPid(pid_t destPid, std::shared_ptr<DeviceHandlerRegistry> registry);
	
	private:
		std::unordered_map<pid_t, std::shared_ptr<ProgRuntimeHandler>> _runtimeInfo;
		int _numberOfProcesses;
		DeviceHandlerFactory &_deviceHandlerFactory;
		const Log _log;

		std::shared_ptr<ProgRuntimeHandler> GetOrAdd(pid_t pid, int status);
		void Drop(pid_t pid);
		bool Step();
};

