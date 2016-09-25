#include "ConfigHandler.h"
#include <iostream>
#include <sstream>
#include "Log.h"

using namespace std;

ConfigHandler::ConfigHandler(const char *path)
{
	_path = path;
}

const ConfigSettings ConfigHandler::LoadConfig()
{
	config_t cfg;
	const char *str;
	ConfigSettings configSettings;
	configSettings.ok = false;
	Log log("config");

	config_init(&cfg);

	if(config_read_file(&cfg, _path) == CONFIG_FALSE)
	{
		log.Error("Line", config_error_line(&cfg), ", msg:", config_error_text(&cfg));
		config_destroy(&cfg);
		return configSettings;
	}

	const config_setting_t *devices = config_lookup(&cfg, "devices");
	if(devices == NULL)
	{
		log.Error("No devices section");
		config_destroy(&cfg);
		return configSettings;
	}

	const int devcount = config_setting_length(devices);

	for(int i = 0;i < devcount;i++)
	{
		const config_setting_t *device = config_setting_get_elem(devices, i);
		if(device == NULL)
		{
			log.Error("Invalid device element");
			config_destroy(&cfg);
			return configSettings;
		}

		const char *name = config_setting_name(device);
		if(name == NULL)
		{
			log.Error("Invalid device name");
			config_destroy(&cfg);
			return configSettings;
		}

		const char *devs = config_setting_get_string(device);
		if(devs == NULL)
		{
			log.Error("Invalid device parameters");
			config_destroy(&cfg);
			return configSettings;
		}
		Device dev;
		dev.name = string(name);
		string s(devs);
		istringstream ss(s);
		string tmp;
		while(ss >> tmp)
			dev.devices.push_back(tmp);
		
		configSettings.devices.push_back(dev);
	}

	const config_setting_t *apps = config_lookup(&cfg, "apps");
	if(apps == NULL)
	{
		log.Error("No apps section");
		config_destroy(&cfg);
		return configSettings;
	}
	const int appcount = config_setting_length(apps);

	for(int i =0;i < appcount;i++)
	{
		const config_setting_t *appElem = config_setting_get_elem(apps, i);
		if(appElem == NULL)
		{
			log.Error("Invalid apps element");
			config_destroy(&cfg);
			return configSettings;
		}
		App app;
		const char *appName = config_setting_name(appElem);
		if(appName == NULL)
		{
			log.Error("Invalid app name");
			config_destroy(&cfg);
			return configSettings;
		}
		app.name = string(appName);
		if(config_setting_lookup_string(appElem, "cmds", &str) == CONFIG_FALSE)
		{
			log.Error("Invalid app cmds");
			config_destroy(&cfg);
			return configSettings;
		}
		string sc(str);
		istringstream ssc(sc);
		string tmp;
		while(ssc >> tmp)
			app.cmds.push_back(tmp);

		if(config_setting_lookup_string(appElem, "devices", &str) == CONFIG_FALSE)
		{
			log.Error("Invalid app devices");
			config_destroy(&cfg);
			return configSettings;
		}
		string sd(str);
		istringstream ssd(sd);
		while(ssd >> tmp)
			app.devices.push_back(tmp);

		configSettings.apps.insert(pair<string, App>(app.name, app));
	}

	config_destroy(&cfg);
	configSettings.ok = true;

	log.Info("Config loaded successfully");

	return configSettings;
}
