#pragma once

#include <string>

using namespace std;

class ArgsParser
{
	public:
		ArgsParser(int argc, char **args);

		bool Parse();

		string &GetAppName();
	private:

		void DisplayHelp() const;

		int _argc;
		char **_args;
		string _appName; 
};
