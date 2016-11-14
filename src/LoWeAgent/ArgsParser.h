#pragma once

#include <string>

using namespace std;

class ArgsParser
{
	public:
		ArgsParser(int argc, char **args);

		bool Parse();

		string GetProgMode() const;
		bool HasProgMode() const;
		bool IsCatchAll() const;
		string GetProgToExec() const;
		bool IsExec() const;
	private:

		void DisplayHelp() const;

		int _argc;
		char **_args;

		string _progMode; 
		bool _isCatchAll;
		string _progToExec;
};
