#include "DeviceHandler.h"
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>

DeviceHandler::DeviceHandler(const int pid, const char *openpath, const string logName): 
	_openpath(openpath), _pid(pid), _log(Log(logName, pid)) 
{
	_syscallbefore = SYS_open;
	_fd = -1;
}

DeviceHandler::~DeviceHandler()
{
}

void DeviceHandler::SetFd(const long fd)
{
	_fd = fd;
	_log.SetFd(fd);
}

void DeviceHandler::PokeData(long addr, char *data, int len) const {
	for(int i = 0;i < len;i+=sizeof(long)) {
		ptrace(PTRACE_POKEDATA, _pid, addr + i, *(long *)(char *)&data[i]);
	}
}

void DeviceHandler::PeekData(long addr, char *out, int len) const {
	long data;
	for(int i = 0;i < len;i+=sizeof(long)) {
		data = ptrace(PTRACE_PEEKDATA, _pid, addr + i, 0);
		*(long *)(&out[i]) = data;
	}
}

bool DeviceHandler::IsDeviceAvailable()
{
	return HasPermissions();
}

string DeviceHandler::GetFixupScript() const
{
	return GetFixupScriptCore();
}

bool DeviceHandler::HasPermissions() const
{
	struct stat s;
	int ret = stat(_openpath.c_str(), &s);
	if(ret < 0)
	{
		_log.Error("File cannot be accessed");
		return false;
	}

	if(access(_openpath.c_str(), R_OK|W_OK) < 0)
	{
		_log.Error("File cannot be read or written");
		return false;
	}

	_log.Info("Device check ok");
	return true;
}

string DeviceHandler::GetFixupScriptCore() const
{
	size_t idx = _openpath.find_last_of("/");
	string dirname = _openpath.substr(0, idx);

	string result = "";
	struct stat s;
	int ret = stat(dirname.c_str(), &s);
	if(ret < 0)
	{
		result+= "mkdir -p " + dirname + "\n";
	}
	result += "> " + _openpath + "\n";
	result += "chmod 666 " + _openpath + "\n";	
	
	return result;
}

