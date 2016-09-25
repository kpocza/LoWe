#pragma once

#include <iostream>
#include <string>

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
        Log(const string name) : _name(name)
        {
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
                        cout << "[DEBUG]{" << _name << "}:";
                        Internal(args...);
                        cout << endl;
                }
        }
 
        template<typename ...Args>
        void Info(const Args&... args) const
        {
                if ((int)_logLevel >= (int)LogLevel::Info)
                {
                        cout << "[INFO]{" << _name << "}:";
                        Internal(args...);
                        cout << endl;
                }
        }
 
        template<typename ...Args>
        void Error(const Args&... args) const
        {
                if ((int)_logLevel >= (int)LogLevel::Error)
                {
                        cout << "[ERR]{" << _name << "}:";
                        Internal(args...);
                        cout << endl;
                }
        }
 
private:
        const string _name;
        static LogLevel _logLevel;
 
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
