#include "PidGuesser.h"
#include "ConfigHandler.h"
#include "DeviceHandlerFactory.h"
#include "DeviceProvisioner.h"
#include "ProgRuntimeDispatcher.h"
#include "Log.h"
#include "ArgsParser.h"
#include "SigActions.h"
#include <iostream>
#include <algorithm>
#include <memory>

LogLevel Log::_logLevel = LogLevel::Info;
ostream *Log::_logout = &cout;

int main(int argc, char **args) 
{
	Log log("main");

	ArgsParser argsParser(argc, args);
	if(!argsParser.Parse())
		return 1;

	ConfigHandler configHandler("loweagent.conf");
	ConfigSettings configSettings = configHandler.LoadConfig();

	if(!configSettings.ok) 
	{
		log.Error("Unable to load config");
		return 1;
	}

	map<string, App>::const_iterator appIt;

	if(argsParser.HasProgMode())
	{
		string progMode = argsParser.GetProgMode();
		log.Info("Determining program mode from parameter:", progMode);
		appIt = configSettings.apps.find(progMode);
	}
	else
	{
		string progToExec = argsParser.GetProgToExec();
		log.Info("Determining program mode from exec param:", progToExec);

		string programName = PidGuesser::GetProgramName(progToExec);
		if(programName.empty())
		{
			log.Error("Unable to determine program mode from exec");
			return 1;
		}
		
		log.Info("Main program name is", programName);
		appIt = std::find_if(configSettings.apps.begin(), configSettings.apps.end(), 
			[programName](const std::pair<string, App> &a) -> bool {
				for(list<string>::const_iterator i = a.second.cmds.begin();i!= a.second.cmds.end();i++)
				{
					if(programName == *i)
						return true;
				}
				return false;
			});
	}

	if(appIt == configSettings.apps.end())
	{
		log.Error("Unable to resolve app config");
		return 1;
	}

	App app = appIt->second;
	log.Info("App config identified");

	if(argsParser.IsCatchAll())
	{
		log.Info("Adding catchall handler");
		app.devices.push_back("catchall");
	}

	DeviceHandlerFactory deviceHandlerFactory;
	deviceHandlerFactory.Configure(app.devices, configSettings.devices);
	log.Info("Device handler configured");

	DeviceProvisioner deviceProvisioner(deviceHandlerFactory);

	if(!deviceProvisioner.EnsureExposer(app.devices, configSettings.appSettings.port))
	{
		log.Error("Unable to ensure that exposer is functioning");
		return 1;
	}

	log.Info("Checking the availability of all devices...");	
	if(!deviceProvisioner.CheckAvailability(app.devices))
	{
		log.Error("One or more devices are not available");
		string fixupScript = deviceProvisioner.GetFixupScript();
		if(fixupScript.size() > 0)
		{
			log.Info("Dev file fixup script is the following:\n", fixupScript);
			log.Info("Executing script as root");
			deviceProvisioner.ExecuteFixupScript();

			log.Info("Script done. Rechecking device availability...");
			if(!deviceProvisioner.CheckAvailability(app.devices))
			{
				log.Error("Unable to recover. Exiting.");
				return 1;
			}
		}
		else
		{
			log.Error("Unable to recover. Exiting.");
			return 1;
		}
	}

	bool isExec = argsParser.IsExec();
	PidGuesser pidGuesser;
	pid_t pid = -1;
	log.Info("Waiting for process to start...");

	if(!isExec)
	{
		pid = pidGuesser.WaitForPid(app.cmds);
	}
	else
	{
		string progToExec = argsParser.GetProgToExec();
		log.Info(progToExec, "will be started");
		pid = pidGuesser.StartProcess(progToExec);
		SigActions::InstallTerminationHandlers(pid);
	}

	if(pid < 0) 
	{
		log.Error("Something went wrong. Cannot determine program's pid");
		return 1;
	}

	log.Info("PID:", pid);

	auto runtimeDispatcher = std::make_shared<ProgRuntimeDispatcher>(deviceHandlerFactory);
	if(!runtimeDispatcher->Init(pid, isExec))
	{
		log.Error("Error initializing main loop");
		return 1;
	}

	log.Info("Spinning...");
	runtimeDispatcher->Spin();
	return 0;
}

