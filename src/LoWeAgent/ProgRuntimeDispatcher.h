#pragma once

#include <sys/user.h>
#include "ProgRuntimeHandler.h"
#include "DeviceHandlerFactory.h"
#include "Log.h"
#include <map>

using namespace std;

class ProgRuntimeDispatcher
{
	public:
		ProgRuntimeDispatcher(DeviceHandlerFactory &deviceHandlerFactory);
		bool Init(pid_t pid, bool isExec);
		bool Spin();
	
	private:
		map<pid_t, ProgRuntimeHandler*> _runtimeInfo;
		int _numberOfProcesses;
		DeviceHandlerFactory &_deviceHandlerFactory;
		const Log _log;

		ProgRuntimeHandler *GetOrAdd(pid_t pid, int status);
		void Drop(pid_t pid);
		bool Step();
};

