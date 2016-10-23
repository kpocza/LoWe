#include "CommunicatingDeviceHandler.h"
#include <algorithm>
#include <string.h>

CommunicatingDeviceHandler::CommunicatingDeviceHandler(const pid_t pid, const string openpath, 
	const string logName, const string id):	DeviceHandler(pid, openpath, logName, id)
{
}

bool CommunicatingDeviceHandler::IsDeviceAvailable()
{
	if(!HasPermissions())
		return false;

	int port = GetPort();
	if(!_socketCommunicator.Open("127.0.0.1", port))
	{
		_log.Error("Socket cannot be opened.");
		_log.Error("Please ensure that LoWeExposer.exe application is running and listening on port", port);
		_socketCommunicator.Close();
		return false;
	}
	if(!SendOpcode(_exposerId))
	{
		_log.Error("Scket cannot be written");
		_log.Error("Please ensure that LoWeExposer.exe application is running and listening on port", port);
		_socketCommunicator.Close();
		return false;
	}

	string rev(_exposerId);
	std::reverse(rev.begin(), rev.end());

	char resp[4]={0};
	if(!_socketCommunicator.Recv((char *)&resp, 4) || strncmp(resp, rev.c_str(), 4))
	{
		_log.Error("Socket cannot be read or bad result");
		_log.Error("Please ensure that LoWeExposer.exe application is running and listening on port", port);
		_socketCommunicator.Close();
		return false;
	}
	_log.Debug("Ok");
	_socketCommunicator.Close();
	return true;
}

int CommunicatingDeviceHandler::SendOpcode(const string opCode)
{
	return _socketCommunicator.Send(opCode.c_str(), 4);
}

