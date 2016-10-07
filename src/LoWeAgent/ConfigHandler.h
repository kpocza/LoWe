#pragma once

#include <libconfig.h>
#include <list>
#include <string>
#include <map>
#include <limits.h>

using namespace std;

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
	list<Device> devices;
	map<string, App> apps;
};

class ConfigHandler
{
	public:
		ConfigHandler(const char *path);

		const ConfigSettings LoadConfig();

	private: 
		bool DetermineConfigPath();

		const char *_path;
		char _fullPath[PATH_MAX];
};
