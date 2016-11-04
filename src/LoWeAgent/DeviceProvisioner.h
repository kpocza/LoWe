#pragma once

#include "DeviceHandlerFactory.h"

class DeviceProvisioner
{
	public:
		DeviceProvisioner(const DeviceHandlerFactory &deviceHandlerFactory);

		bool EnsureExposer(const list<string> &devicesToSpy, const int port);
		bool CheckAvailability(const list<string> &devicesToSpy);
		string GetFixupScript() const;
		void ExecuteFixupScript() const;

	private:
		const DeviceHandlerFactory &_deviceHandlerFactory;
		string _fixupScript;
};
