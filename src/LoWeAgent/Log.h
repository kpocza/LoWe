#pragma once

#include <iostream>
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
 
	template<typename ...Args>
	void Debug(const Args&... args) const
	{
		if ((int)_logLevel >= (int)LogLevel::Debug)
		{
			Prefix("DEBUG");
			Internal(args...);
			cout << endl;
		}
	}
 
	template<typename ...Args>
	void Info(const Args&... args) const
	{
		if ((int)_logLevel >= (int)LogLevel::Info)
		{
			Prefix("INFO");
			Internal(args...);
			cout << endl;
		}
	}
 
	template<typename ...Args>
	void Error(const Args&... args) const
	{
		if ((int)_logLevel >= (int)LogLevel::Error)
		{
			Prefix("ERR");
			Internal(args...);
			cout << endl;
		}
	}
 
private:
	const string _name;
	const pid_t _pid;
	long _fd;
	static LogLevel _logLevel;

	void Prefix(const string& level) const
	{
		cout << "[" << level << "]";
		if(_pid!= -1)
		{
			cout << "[P " << _pid << "]";
		}
		if(_fd!= -1)
		{
			cout << "[FD " << _fd << "]";
		}
		cout << "{" << _name << "}:";
	}
 
	template<typename Arg>
	void Internal(const Arg& arg) const
	{
		cout << " " << arg;
	}
 
	template<typename Arg, typename ...Args>
	void Internal(const Arg& arg, const Args&... args) const
	{
		Internal(arg);
		Internal(args...);
	}
};
