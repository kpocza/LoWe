#pragma once

#include "DeviceHandler.h"
#include "SocketCommunicator.h"
#include <map>

class CommunicatingDeviceHandler : public DeviceHandler
{
	protected:
		CommunicatingDeviceHandler(const pid_t pid, const string openpath, const string logName, 
			const string exposerId);

	public:
		virtual bool IsDeviceAvailable() override;
		void SetPort(int port);
		int GetPort() const;

	protected:
		int SendOpcode(const string opcode);

		SocketCommunicator _socketCommunicator;

	private:
		static map<string, int> _ports;
};
