#pragma once

#include <string>

using namespace std;

class ArgsParser
{
	public:
		ArgsParser(int argc, char **args);

		bool Parse();
		bool IsCatchAll();

		string &GetAppName();
	private:

		void DisplayHelp() const;

		int _argc;
		char **_args;
		string _appName; 
		bool _isCatchAll;
};
