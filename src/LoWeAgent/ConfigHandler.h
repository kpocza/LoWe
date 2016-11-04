#pragma once

#include <libconfig.h>
#include <list>
#include <string>
#include <map>
#include <limits.h>

using namespace std;

struct AppSettings
{
	int port;
};

struct Device
{
	string name;
	list<string> devices;
};

struct App
{
	string name;
	list<string> cmds;
	list<string> devices;
};

struct ConfigSettings
{
	bool ok;
	AppSettings appSettings;
	list<Device> devices;
	map<string, App> apps;
};

class ConfigHandler
{
	public:
		ConfigHandler(const char *path);

		const ConfigSettings LoadConfig();

	private: 
		void DetermineConfigPath();

		const char *_path;
		char _fullPath[PATH_MAX];
};
