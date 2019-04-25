#pragma once

#include <sys/user.h>
#include <string>
#include "Log.h"
#include <unordered_map>
#include <memory>

class DeviceHandler;

typedef std::unordered_map<long, std::shared_ptr<DeviceHandler>> HandleMap;

class DeviceHandler
{
	protected:
		DeviceHandler(const int pid, const string openpath, const string logName, const string exposerId);

	public:
		virtual ~DeviceHandler();
		virtual bool IsDeviceAvailable();
		virtual string GetFixupScript() const;
		virtual void ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs)=0;
		virtual void ExecuteAfter(pid_t pid, const long syscall, user_regs_struct &regs)=0;

		void SetFd(const long fd);
		string GetExposerId() const;

	protected:
		const std::string _openpath;
		Log _log;
		const string _exposerId;
		
		
		long _fd;
		long _syscallbefore;
		long _syscallafter;

		long GetTimeMillisec();
		void PokeData(pid_t pid, long addr, const void *dataInput, int len) const;
		void PeekData(pid_t pid, long addr, void *dataOutput, int len) const;
		bool HasPermissions() const;
		string GetFixupScriptCore() const;
};
