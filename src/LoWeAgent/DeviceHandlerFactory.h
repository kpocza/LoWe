#pragma once

#include "DeviceHandler.h"
#include "ConfigHandler.h"
#include "Log.h"

#include <memory>

class DeviceHandlerFactory
{
	public:
		DeviceHandlerFactory();

		void Configure(const list<string> &devicesToSpy, const list<Device> &devices);
		std::shared_ptr<DeviceHandler> Create(const string path, const pid_t pid) const;

	private:
		std::shared_ptr<DeviceHandler> CreateInternal(const string &path, const string &id, const pid_t pid) const;

		const Log _log;

		bool _catchAll;
		map<string, string> _spyConfig;
};
