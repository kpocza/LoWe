#include "DeviceHandlerRegistry.h"

#include <iostream>

using namespace std;

DeviceHandlerRegistry::DeviceHandlerRegistry()
{
}

void DeviceHandlerRegistry::Register(const long fd, std::shared_ptr<DeviceHandler> deviceHandler)
{
	Unregister(fd);	

	deviceHandler->SetFd(fd);
	_deviceHandlers.emplace(fd, deviceHandler);
}

void DeviceHandlerRegistry::Unregister(const long fd)
{
	if(_deviceHandlers.find(fd)!= _deviceHandlers.end())
	{
		_deviceHandlers.erase(fd);
	}
}

std::shared_ptr<DeviceHandler> DeviceHandlerRegistry::Lookup(const long fd)
{
	auto it = _deviceHandlers.find(fd);

	if(it!= _deviceHandlers.end())
	{
		return it->second;
	}
	return NULL;
}

void DeviceHandlerRegistry::AddProcessRelationship(const long childPid, const long pid)
{
	_processRelationship[childPid] = pid;
}

bool DeviceHandlerRegistry::OneTimeDuplicateHandles(const std::shared_ptr<ProgRuntimeHandler>& runtimeHandler)
{
	return runtimeHandler->AcceptHandlesFromOtherProcess(_deviceHandlers);
}

bool DeviceHandlerRegistry::RegisterMap(const HandleMap& deviceHandlers)
{
	_deviceHandlers.insert(deviceHandlers.begin(), deviceHandlers.end());

	return true;
}

