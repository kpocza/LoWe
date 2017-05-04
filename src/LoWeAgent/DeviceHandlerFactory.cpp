#include "DeviceHandlerFactory.h"
#include "DeviceHandlerFrameBuffer.h"
#include "DeviceHandlerTTY.h"
#include "DeviceHandlerALSA.h"
#include "DeviceHandlerCatchAll.h"
#include "DeviceHandlerEvMice.h"
#include "DeviceHandlerEvKeyboard.h"
#include <string.h>
#include <iostream>

using namespace std;

DeviceHandlerFactory::DeviceHandlerFactory(): _log(Log("factory"))
{
}

void DeviceHandlerFactory::Configure(const list<string> &devicesToSpy, const list<Device> &devices)
{
	_catchAll = false;
	for(list<string>::const_iterator dsi = devicesToSpy.begin();dsi!= devicesToSpy.end();dsi++)
	{
		string devPath(*dsi);

		if(devPath == "catchall")
		{
			_catchAll = true;
			continue;
		}
	
		bool found = false;
		for(list<Device>::const_iterator di = devices.begin();di!= devices.end() && !found;di++)
		{
			for(list<string>::const_iterator dni = di->devices.begin();
				dni!= di->devices.end() && !found;dni++)
			{
				if(devPath == *dni)
				{
					_spyConfig.insert(pair<string, string>(devPath, di->name));
					found = true;
				}
			}
		}
	}
}

DeviceHandler *DeviceHandlerFactory::Create(const string path, const pid_t pid) const
{
	_log.Debug("Opening:", path);

	map<string, string>::const_iterator handlerId = _spyConfig.find(path);
	if(handlerId == _spyConfig.end())
	{
		if(_catchAll)
			return new DeviceHandlerCatchAll(pid, path);

		return NULL;
	}

	return CreateInternal(path, handlerId->second, pid);
}

DeviceHandler *DeviceHandlerFactory::CreateInternal(const string &path, const string &id, const pid_t pid) const
{
	if(id == "fb")
		return new DeviceHandlerFrameBuffer(pid, path);
	if(id == "tty")
		return new DeviceHandlerTTY(pid, path);
	if(id == "alsa")
		return new DeviceHandlerALSA(pid, path);
	if(id == "evmice")
		return new DeviceHandlerEvMice(pid, path);
	if(id == "evkbd")
		return new DeviceHandlerEvKeyboard(pid, path);

	return NULL;
}

