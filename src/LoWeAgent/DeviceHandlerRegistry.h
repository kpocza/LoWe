#pragma once

#include "DeviceHandler.h"
#include "ProgRuntimeDispatcher.h"
#include <map>

using namespace std;

class DeviceHandlerRegistry
{
	public:
		DeviceHandlerRegistry(ProgRuntimeDispatcher &progRuntimeDispatcher);

		void Register(const long fd, DeviceHandler *deviceHandler);
		void Unregister(const long fd);
		DeviceHandler *Lookup(const long fd);
		void AddProcessRelationship(const long childPid, const long pid);

	private:
		map<long, DeviceHandler*> _deviceHandlers;
		map<long, long> _processRelationship;
		ProgRuntimeDispatcher &_progRuntimeDispatcher;
};
