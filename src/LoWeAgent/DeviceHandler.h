#pragma once

#include <sys/user.h>
#include <string>
#include "Log.h"

class DeviceHandler
{
	protected:
		DeviceHandler(const pid_t pid, const string openpath, const string logName, const string exposerId);

	public:
		virtual ~DeviceHandler();
		virtual bool IsDeviceAvailable();
		virtual string GetFixupScript() const;
		virtual void ExecuteBefore(const long syscall, user_regs_struct &regs)=0;
		virtual void ExecuteAfter(const long syscall, user_regs_struct &regs)=0;

		void SetFd(const long fd);
		string GetExposerId() const;

	protected:
		const std::string _openpath;
		const pid_t _pid;
		Log _log;
		const string _exposerId;
		
		
		long _fd;
		long _syscallbefore;
		long _syscallafter;

		long GetTimeMillisec();
		void PokeData(long addr, char *data, int len) const;
		void PeekData(long addr, char *out, int len) const;
		bool HasPermissions() const;
		string GetFixupScriptCore() const;
};
