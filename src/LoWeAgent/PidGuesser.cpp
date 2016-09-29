#include "PidGuesser.h"
#include "Log.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <fstream>
#include <sstream>
#include "Log.h"

using namespace std;

PidGuesser::PidGuesser(list<string> cmds)
{
	_cmds = cmds;
}

pid_t PidGuesser::GetPid()
{
//	_dir = opendir("/proc");
//	if(_dir == NULL || errno)
//	{
//		return -1;
//	}

	pid_t pid;
	do
	{
		pid = Findpid();
		usleep(50);
	} 
	while(pid == -1);

//	closedir(_dir);

	return pid;
}


bool PidGuesser::IsNumeric(const char *chars) 
{
	for(;*chars;chars++) {
		if(*chars < '0' || *chars > '9')
			return false;
	}
	return true;
}


pid_t PidGuesser::Findpid()
{
	char buf[128], pidname[6]={0}, procname[100];
	FILE *fp = popen("ps -e", "r");
	if(fp == NULL)
		return -1;

	while(fgets(buf, 128, fp)!= NULL)
	{
		strncpy(pidname, buf, 5);
		int pid = atoi(pidname);
		if(pid <= 0)
			continue;

		strncpy(procname, buf+24, 100);
		procname[strlen(procname)-1] = '\0';

		string procstr(procname);

		for(list<string>::const_iterator i = _cmds.begin();i!= _cmds.end();i++)
		{
			if(procstr == *i)
				return pid;
		}
	}	

	pclose(fp);

	return -1;
/*	struct dirent* de;

	rewinddir(_dir);
	Log log("fp");
	log.Info("start");
	log.Info(errno);
	while((de = readdir(_dir)))
	{
		if(errno)
		{
			log.Info(errno);
			break;
		}
		if(de->d_type!= DT_DIR)
			continue;

		if(!IsNumeric(de->d_name))
			continue;

		string cmdlinePath = "/proc/" + string(de->d_name) + "/cmdline";
		string firstLine, procName;

		ifstream cmdFile(cmdlinePath);
		if(!getline(cmdFile, procName, '\0'))
		{
			cmdFile.close();
			break;
		}
		cmdFile.close();

		for(list<string>::const_iterator i = _cmds.begin();i!= _cmds.end();i++)
		{
			if(procName == *i)
				return atoi(de->d_name);
		}
	}*/

	return -1;
}


