#include "DeviceHandlerRegistry.h"

#include <iostream>

using namespace std;

DeviceHandlerRegistry::DeviceHandlerRegistry()
{
}

void DeviceHandlerRegistry::Register(const long fd, DeviceHandler *deviceHandler)
{
	if(_deviceHandlers.find(fd)!= _deviceHandlers.end())
	{
		Unregister(fd);	
	}

	deviceHandler->SetFd(fd);
	_deviceHandlers.insert(pair<long, DeviceHandler*>(fd, deviceHandler));
}

void DeviceHandlerRegistry::Unregister(const long fd)
{
	_deviceHandlers.erase(fd);
}

DeviceHandler *DeviceHandlerRegistry::Lookup(const long fd)
{
	map<long, DeviceHandler*>::iterator it = _deviceHandlers.find(fd);

	if(it!= _deviceHandlers.end())
	{
		return it->second;
	}
	return NULL;
}

