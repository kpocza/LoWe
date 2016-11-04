#include <fstream>
#include <set>
#include "DeviceProvisioner.h"
#include "CommunicatingDeviceHandler.h"
#include "SocketCommunicator.h"
#include "Log.h"
#include <string.h>

DeviceProvisioner::DeviceProvisioner(const DeviceHandlerFactory &deviceHandlerFactory) :
	_deviceHandlerFactory(deviceHandlerFactory)
{
}

bool DeviceProvisioner::EnsureExposer(const list<string> &devicesToSpy, const int port)
{
	Log log("checkexposer");

	log.Info("Ensuring LoWeExposer handled devices...");
	set<string> exposerIds;
	for(list<string>::const_iterator path = devicesToSpy.begin();path!= devicesToSpy.end();path++)
	{
		DeviceHandler *deviceHandler = _deviceHandlerFactory.Create(*path, -1);

		string exposerId = deviceHandler->GetExposerId();
		if(exposerId.size() == 4)
		{
			exposerIds.insert(exposerId);
		}
	}

	SocketCommunicator socketCommunicator;

	log.Info("Connecting to LoWeExposer on port no.", port);
	if(!socketCommunicator.Open("127.0.0.1", port))
	{
		log.Error("Socket cannot be opened");
		log.Error("Please ensure that LoWeExposer.exe application is running");
		socketCommunicator.Close();
		return false;
	}

	string exposerIdString;

	for(set<string>::const_iterator ci = exposerIds.begin(); ci!= exposerIds.end();ci++)
	{
		exposerIdString+= *ci;
	}

	int count = exposerIds.size();
	socketCommunicator.Send("LOWE", 4);
	socketCommunicator.Send((char *)&count, 4);
	socketCommunicator.Send(exposerIdString.c_str(), exposerIdString.size());

	char respBuffer[4+count*4*2];
	if(!socketCommunicator.Recv(respBuffer, 4+count*4*2))
	{
		log.Error("Unable to receive data");
		log.Error("Please ensure that LoWeExposer.exe application is running");
		socketCommunicator.Close();
		return false;
	}

	if(strncmp(respBuffer, "EWOL", 4))
	{
		log.Error("Invalid response header");
		log.Error("Please ensure that LoWeExposer.exe application is running");
		socketCommunicator.Close();
		return false;
	}

	map<string, int> exposerInfo;

	char *respIdx = respBuffer+4;
	while(count > 0)
	{
		char expId[5]={0};
		strncpy(expId, respIdx, 4);
		string expIdStr = expId;
		int expInfo = *(int *)&respIdx[4];
		log.Debug("Exposer setup response", expIdStr, "-", expInfo);
		exposerInfo.insert(pair<string, int>(expIdStr, expInfo));
		respIdx+=8;
		count--;
	}

	for(list<string>::const_iterator path = devicesToSpy.begin();path!= devicesToSpy.end();path++)
	{
		DeviceHandler *deviceHandler = _deviceHandlerFactory.Create(*path, -1);

		string exposerId = deviceHandler->GetExposerId();
		if(exposerId.size() != 4)
			continue;

		map<string, int>::iterator response = exposerInfo.find(exposerId);
		if(response == exposerInfo.end())
		{
			log.Error("Invalid exposer id:", exposerId);
			log.Error("Please ensure that LoWeExposer.exe application is running");
			return false;
		}

		if(response->second <= 0)
		{
			log.Error("Invalid device handler port or status");
			log.Error("Please ensure that LoWeExposer.exe application is running");
			return false;
		}

		CommunicatingDeviceHandler *dhc = dynamic_cast<CommunicatingDeviceHandler*>(deviceHandler);
		if(dhc != NULL)
		{
			log.Debug("Setting up port", response->second, "for", exposerId);
			dhc->SetPort(response->second);
		}
	}
	return true;
}

bool DeviceProvisioner::CheckAvailability(const list<string> &devicesToSpy)
{
	Log log("checker");

	_fixupScript = "";

	bool allOk = true;
	for(list<string>::const_iterator path = devicesToSpy.begin();path!= devicesToSpy.end();path++)
	{
		DeviceHandler *deviceHandler = _deviceHandlerFactory.Create(*path, -1);
		log.Debug("Examining device:", deviceHandler->GetExposerId());
		if(!deviceHandler->IsDeviceAvailable())
		{
			_fixupScript+= deviceHandler->GetFixupScript();
			allOk = false;
		}
		delete deviceHandler;
	}

	return allOk;
}

string DeviceProvisioner::GetFixupScript() const
{
	return _fixupScript;
}

void DeviceProvisioner::ExecuteFixupScript() const
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

