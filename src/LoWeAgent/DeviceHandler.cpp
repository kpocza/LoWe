#include "DeviceHandler.h"
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

DeviceHandler::DeviceHandler(const int pid, const string openpath, const string logName, const string exposerId): 
	_openpath(openpath), _log(Log(logName, pid)), _exposerId(exposerId) 
{
	_syscallbefore = SYS_open;
	_fd = -1;
}

DeviceHandler::~DeviceHandler()
{
	_log.Debug("Fully closing fd:", _fd);
}

void DeviceHandler::SetFd(const long fd)
{
	_fd = fd;
	_log.SetFd(fd);
}

void DeviceHandler::PokeData(pid_t pid, long addr, const void *dataInput, int len) const 
{
//	Why does the following code show errno 14 - EFAULT?
/*	struct iovec local, remote;

	local.iov_base = dataInput;
	local.iov_len = len;
	remote.iov_base = (void *)addr;
	remote.iov_len = len;

	process_vm_writev(_pid, &local, 1, &remote, 1, 0);*/
	char *data = (char *)dataInput;
	int chunkSize = sizeof(long);
	int fullChunksEnd = (len/chunkSize)*chunkSize;
	int restLen = len - fullChunksEnd;

	for(int i = 0;i < fullChunksEnd;i+=chunkSize) 
	{
		ptrace(PTRACE_POKEDATA, pid, addr + i, *(long *)&data[i]);
	}

	if(restLen > 0)
	{
		char lastChunk[chunkSize];
		long lastData = ptrace(PTRACE_PEEKDATA, pid, addr + fullChunksEnd, 0);
		*(long *)(&lastChunk) = lastData;
		for(int i = 0;i < restLen;i++)
			lastChunk[i] = data[fullChunksEnd + i];
		ptrace(PTRACE_POKEDATA, pid, addr + fullChunksEnd, *(long *)(char *)&lastChunk);
	}
}

void DeviceHandler::PeekData(pid_t pid, long addr, void *dataOutput, int len) const {
	struct iovec local, remote;

	local.iov_base = dataOutput;
	local.iov_len = len;
	remote.iov_base = (void *)addr;
	remote.iov_len = len;

	process_vm_readv(pid, &local, 1, &remote, 1, 0);
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

long DeviceHandler::GetTimeMillisec()
{
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);

	long s = (long)spec.tv_sec;
	long ms = round(spec.tv_nsec / 1000000);

	return 1000 * s + ms;
}

string DeviceHandler::GetExposerId() const
{
	return _exposerId;
}


