#pragma once

#include "DeviceHandler.h"
#include "ProgRuntimeDispatcher.h"
#include <unordered_map>

#include <memory>

using namespace std;

class ProgRuntimeHandler;

class DeviceHandlerRegistry
{
	public:
		DeviceHandlerRegistry();

		void Register(const long fd, std::shared_ptr<DeviceHandler> deviceHandler);
		bool RegisterMap(const HandleMap& deviceHandlers);
		void Unregister(const long fd);
		std::shared_ptr<DeviceHandler> Lookup(const long fd);
		void AddProcessRelationship(const long childPid, const long pid);

		bool OneTimeDuplicateHandles(const std::shared_ptr<ProgRuntimeHandler>& runtimeHandler);

	private:
		HandleMap _deviceHandlers;
		unordered_map<long, long> _processRelationship;
};
