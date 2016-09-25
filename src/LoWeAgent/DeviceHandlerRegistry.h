#pragma once

#include "DeviceHandler.h"
#include <map>

using namespace std;

class DeviceHandlerRegistry
{
	public:
		DeviceHandlerRegistry();

		void Register(const long fd, DeviceHandler *deviceHandler);
		void Unregister(const long fd);
		DeviceHandler *Lookup(const long fd);

	private:
		map<long, DeviceHandler*> _deviceHandlers;
};
