#include <iostream>
#include "ArgsParser.h"
#include <algorithm>
#include <getopt.h>
#include "Log.h"

ArgsParser::ArgsParser(int argc, char **args): _argc(argc), _args(args)
{
	_isCatchAll = false;
}

bool ArgsParser::Parse()
{
	struct option long_options[] =
	{
		{"outfile", required_argument, 0, 'o'},
		{"loglevel", required_argument, 0, 'l'},
		{"help", no_argument, 0, 'h'},
		{"catchall", no_argument, 0, 'c'},
		{0, 0, 0, 0}
	};

	Log log("args");

	bool fine = true;

	if(_argc == 1)
		fine = false;

	while(1)
	{
		int idx = 0;
		int c = getopt_long_only(_argc, _args, "o:l:", long_options, &idx);

		if(c == -1)
			break;

		switch(c)
		{
			case 'o':
			{
				string logFilePath(optarg);
				Log::SetLogFile(logFilePath);
				break;
			}
			case 'l':
			{
				string logLevel(optarg);
				transform(logLevel.begin(), logLevel.end(), logLevel.begin(), ::toupper);
				log.Info("loglevel:", logLevel);

				if(logLevel == "DEBUG") {
					Log::SetLogLevel(LogLevel::Debug);
				} else if(logLevel == "INFO") {
					Log::SetLogLevel(LogLevel::Info);
				} else if(logLevel == "ERROR") {
					Log::SetLogLevel(LogLevel::Error);
				} else {
					DisplayHelp();
					return false;
				}
				break;
			}
			case 'c':
			{
				_isCatchAll = true;
				break;
			}
			case 'h':
			{
				DisplayHelp();
				return false;
			}
			default:
			{
				fine = false;
				break;
			}
		}
	}
	if(!fine)
	{
		DisplayHelp();
		return false;
	}

	if(optind == _argc - 1)
	{
		_appName = string(_args[optind]);
		log.Info("Appname:", _appName);
	}
	else
	{
		DisplayHelp();
		fine = false;
	}

	return fine;
}

string &ArgsParser::GetAppName()
{
	return _appName;
}

bool ArgsParser::IsCatchAll()
{
	return _isCatchAll;
}

void ArgsParser::DisplayHelp() const
{
	cout << "Usage: " << endl;
	cout << "    loweagent [-o file.log] [-l Debug|Info|Error] [-c] [-h] program_mode" << endl << endl;
	cout << "Description: " << endl;
	cout << "    Listens and reacts to syscalls specified by the program_mode parameter." << endl;
	cout << "    program_mode parameter refers to an item of loweagent.conf that specified which" << endl;
	cout << "    /dev-s are to be tracked and handled." << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << "    -o, -outfile:     Send log messages to this file instread of the stdout" << endl;
	cout << "    -l, -loglevel:    Set Debug, Info or Error loglevel. Default: Info" << endl;
	cout << "    -c, -catchall:    Catch all syscalls that are not handled otherwise and provide log information" 
		<< endl;
	cout << "    -h, -help:        Display this help" << endl;
	cout << endl;
}

