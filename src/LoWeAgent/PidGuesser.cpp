#include "PidGuesser.h"
#include "Log.h"
#include <string.h>
#include <stdio.h>
#include <string>
#include "Log.h"

using namespace std;

PidGuesser::PidGuesser(list<string> cmds)
{
	_cmds = cmds;
}

pid_t PidGuesser::GetPid()
{
	pid_t pid;
	do
	{
		pid = Findpid();
		usleep(50);
	} 
	while(pid == -1);

	return pid;
}

pid_t PidGuesser::Findpid()
{
	char buf[128], pidname[6]={0}, procname[100];
	FILE *fp = popen("ps -e", "r");
	if(fp == NULL)
		return -1;

	while(fgets(buf, 128, fp)!= NULL)
	{
		memset(pidname, 0, sizeof(pidname));
		strncpy(pidname, buf, 5);
		int pid = atoi(pidname);
		if(pid <= 0)
			continue;

		memset(procname, 0, sizeof(procname));
		strncpy(procname, buf+24, 99);
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
}


