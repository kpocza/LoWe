#include "ArgsParser.h"
#include <algorithm>
#include <getopt.h>
#include "Log.h"

ArgsParser::ArgsParser(int argc, char **args): _argc(argc), _args(args)
{
}

bool ArgsParser::Parse()
{
	struct option long_options[] =
	{
		{"loglevel", required_argument, 0, 'l'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	Log log("args");

	bool fine = true;

	if(_argc == 1)
		fine = false;

	while(1)
	{
		int idx = 0;
		int c = getopt_long_only(_argc, _args, "l:", long_options, &idx);

		if(c == -1)
			break;

		switch(c)
		{
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

void ArgsParser::DisplayHelp() const
{
	cout << "Usage: " << endl;
	cout << "   loweagent [-loglevel Debug|Info|Error] prog_to_attach" << endl << endl;
}

