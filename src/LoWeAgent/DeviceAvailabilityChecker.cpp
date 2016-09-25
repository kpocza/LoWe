#include "DeviceAvailabilityChecker.h"
#include "Log.h"
#include <fstream>

DeviceAvailabilityChecker::DeviceAvailabilityChecker(const DeviceHandlerFactory &deviceHandlerFactory) :
	_deviceHandlerFactory(deviceHandlerFactory)
{
}

bool DeviceAvailabilityChecker::Check(const list<string> &devicesToSpy)
{
	Log log("checker");

	_fixupScript = "";

	bool allOk = true;
	for(list<string>::const_iterator path = devicesToSpy.begin();path!= devicesToSpy.end();path++)
	{
		DeviceHandler *deviceHandler = _deviceHandlerFactory.Create(path->c_str(), -1);
		if(!deviceHandler->IsDeviceAvailable())
		{
			_fixupScript+= deviceHandler->GetFixupScript();
			allOk = false;
		}
		delete deviceHandler;
	}

	return allOk;
}

string DeviceAvailabilityChecker::GetFixupScript() const
{
	return _fixupScript;
}

void DeviceAvailabilityChecker::ExecuteFixupScript() const
{
	string tmppath = string(std::tmpnam(nullptr)) + ".sh";

	ofstream out(tmppath);
	out << "# /bin/bash" << endl << endl;
	out << _fixupScript << endl;
	out.close();

	std::system(("chmod u+x " + tmppath).c_str());
	std::system(("sudo " + tmppath).c_str());
	std::remove(tmppath.c_str());
}

