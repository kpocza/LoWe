#include <iostream>
#include "PidGuesser.h"
#include "ConfigHandler.h"
#include "DeviceHandlerFactory.h"
#include "DeviceAvailabilityChecker.h"
#include "ProgRuntimeDispatcher.h"
#include "Log.h"

using namespace std;

LogLevel Log::_logLevel = LogLevel::Info;

int main(int argc, char **args) 
{
	Log log("main");

	if(argc!= 2) 
	{
		cout << "Usage: " << endl;
		cout << "   loweagent prog_to_attach" << endl << endl;
		return 1;
	}

	ConfigHandler configHandler("loweagent.config");
	ConfigSettings configSettings = configHandler.LoadConfig();

	if(!configSettings.ok) 
	{
		log.Error("Unable to load config");
		return 1;
	}

	string appName(args[1]);
	map<string, App>::const_iterator appIt = configSettings.apps.find(appName);

	if(appIt == configSettings.apps.end())
	{
		log.Error("Unable to resolve app config");
		return 1;
	}

	App app = appIt->second;
	log.Info("App config identified");

	DeviceHandlerFactory deviceHandlerFactory;
	deviceHandlerFactory.Configure(app.devices, configSettings.devices);
	log.Info("Device handler configured");

	DeviceAvailabilityChecker deviceAvailabilityChecker(deviceHandlerFactory);
	if(!deviceAvailabilityChecker.Check(app.devices))
	{
		log.Error("One or more devices are not available");
		string fixupScript = deviceAvailabilityChecker.GetFixupScript();
		if(fixupScript.size() > 5)
		{
			log.Info("Dev file fixup script is the following");
	 		cout << fixupScript << endl;
			log.Info("Executing script as root");
			deviceAvailabilityChecker.ExecuteFixupScript();
		}
		return 1;
	}

	log.Info("Waiting for process to start...");	
	PidGuesser pidGuesser(app.cmds);
	pid_t pid = pidGuesser.GetPid();

	log.Info("PID:", pid);

	ProgRuntimeDispatcher runtimeDispatcher(deviceHandlerFactory);

	log.Info("Spinning...");
	runtimeDispatcher.Do(pid);
	return 0;
}

