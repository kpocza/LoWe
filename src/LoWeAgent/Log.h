#pragma once

#include <fstream>
#include <string>
#include <sys/user.h>

using namespace std;

enum LogLevel
{
	Error = 0,
	Info = 1,
	Debug = 2
};
 
class Log
{
public:
	Log(const string name, const pid_t pid = -1) : _name(name), _pid(pid)
	{
		_fd = -1;
	}

	void SetFd(int fd)
	{
		_fd = fd;
	}
 
	static void SetLogLevel(LogLevel logLevel)
	{
		Log::_logLevel = logLevel;
	}

	static void SetLogFile(const string fileName)
	{
		_logout = new ofstream(fileName);
	}
 
	template<typename ...Args>
	void Debug(const Args&... args) const
	{
		if ((int)_logLevel >= (int)LogLevel::Debug)
		{
			Prefix("DEBUG");
			Internal(args...);
			*_logout << endl;
		}
	}
 
	template<typename ...Args>
	void Info(const Args&... args) const
	{
		if ((int)_logLevel >= (int)LogLevel::Info)
		{
			Prefix("INFO");
			Internal(args...);
			*_logout << endl;
		}
	}
 
	template<typename ...Args>
	void Error(const Args&... args) const
	{
		if ((int)_logLevel >= (int)LogLevel::Error)
		{
			Prefix("ERR");
			Internal(args...);
			*_logout << endl;
		}
	}
 
private:
	const string _name;
	const pid_t _pid;
	long _fd;
	static LogLevel _logLevel;
	static ostream *_logout;

	void Prefix(const string& level) const
	{
		*_logout << "[" << level << "]";
		if(_pid!= -1)
		{
			*_logout << "[P " << _pid << "]";
		}
		if(_fd!= -1)
		{
			*_logout << "[FD " << _fd << "]";
		}
		*_logout << "{" << _name << "}:";
	}
 
	template<typename Arg>
	void Internal(const Arg& arg) const
	{
		*_logout << " " << arg;
	}
 
	template<typename Arg, typename ...Args>
	void Internal(const Arg& arg, const Args&... args) const
	{
		Internal(arg);
		Internal(args...);
	}
};
