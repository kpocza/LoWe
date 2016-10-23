#pragma once

#include "DeviceHandler.h"
#include "SocketCommunicator.h"

class CommunicatingDeviceHandler : public DeviceHandler
{
	protected:
		CommunicatingDeviceHandler(const pid_t pid, const string openpath, const string logName, 
			const string id);

	public:
		virtual bool IsDeviceAvailable() override;
		virtual void SetPort(int port)=0;
		virtual int GetPort() const =0;

	protected:
		int SendOpcode(const string opcode);

		SocketCommunicator _socketCommunicator;
};
