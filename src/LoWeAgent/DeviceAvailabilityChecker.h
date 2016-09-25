#pragma once

#include "DeviceHandlerFactory.h"

class DeviceAvailabilityChecker
{
	public:
		DeviceAvailabilityChecker(const DeviceHandlerFactory &deviceHandlerFactory);

		bool Check(const list<string> &devicesToSpy);
		string GetFixupScript() const;
		void ExecuteFixupScript() const;

	private:
		const DeviceHandlerFactory &_deviceHandlerFactory;
		string _fixupScript;
};
