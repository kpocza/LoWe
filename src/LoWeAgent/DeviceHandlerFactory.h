#pragma once

#include "DeviceHandler.h"
#include "ConfigHandler.h"
#include "Log.h"

class DeviceHandlerFactory
{
	public:
		DeviceHandlerFactory();

		void Configure(const list<string> &devicesToSpy, const list<Device> &devices);
		DeviceHandler *Create(const string path, const pid_t pid) const;

	private:
		DeviceHandler *CreateInternal(const string &path, const string &id, const pid_t pid) const;

		const Log _log;

		bool _catchAll;
		map<string, string> _spyConfig;
};
