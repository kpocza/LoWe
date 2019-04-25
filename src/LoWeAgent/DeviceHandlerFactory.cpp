#include "DeviceHandlerFactory.h"
#include "DeviceHandlerFrameBuffer.h"
#include "DeviceHandlerTTY.h"
#include "DeviceHandlerALSA.h"
#include "DeviceHandlerCatchAll.h"
#include "DeviceHandlerEvMice.h"
#include "DeviceHandlerEvKeyboard.h"
#include "DeviceHandlerSysDirectory.h"
#include <string.h>
#include <iostream>
#include <memory>

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

std::shared_ptr<DeviceHandler> DeviceHandlerFactory::Create(const string path, const pid_t pid) const
{
	_log.Debug("Opening:", path);

	map<string, string>::const_iterator handlerId = _spyConfig.find(path);
	if(handlerId == _spyConfig.end())
	{
		#if 0
		if(path == "char") {
			return new DeviceHandlerSysDirectory(pid, path);
		}

		if(path == "tmp") {
			return new DeviceHandlerSysDirectory(pid, path);
		}

		if(path.find("/sys/dev//tmp/char") == 0) {
			// Deal with anything along this path
			return new DeviceHandlerSysDirectory(pid, path);
		}

		if(path.find("/run/udev/data/c13") == 0) {
			// Deal with anything along this path
			return new DeviceHandlerSysDirectory(pid, path);
		}
		#endif

		if(_catchAll)
			return std::make_shared<DeviceHandlerCatchAll>(pid, path);

		return NULL;
	}

	return CreateInternal(path, handlerId->second, pid);
}

std::shared_ptr<DeviceHandler>  DeviceHandlerFactory::CreateInternal(const string &path, const string &id, const pid_t pid) const
{
	if(id == "fb") {
		_log.Debug("new DeviceHandlerFrameBuffer:", path);
		return std::make_shared<DeviceHandlerFrameBuffer>(DeviceHandlerFrameBuffer(pid, path));
	}
	if(id == "tty") {
		_log.Debug("new DeviceHandlerTTY:", path);
		return std::make_shared<DeviceHandlerTTY>(pid, path);
	}
	if(id == "alsa") {
		_log.Debug("new DeviceHandlerALSA:", path);
		return  std::make_shared<DeviceHandlerALSA>(pid, path);
	}
	if(id == "evmice") {
		_log.Debug("new DeviceHandlerEvMice:", path);
		return  std::make_shared<DeviceHandlerEvMice>(pid, path);
	}
	if(id == "evkbd") {
		_log.Debug("new DeviceHandlerEvKeyboard:", path);
		return std::make_shared<DeviceHandlerEvKeyboard>(pid, path);
	}
	if(id == "sys") {
		_log.Debug("new DeviceHandlerSysDirectory:", path);
		return  std::make_shared<DeviceHandlerSysDirectory>(pid, path);
	}

	return nullptr;
}

