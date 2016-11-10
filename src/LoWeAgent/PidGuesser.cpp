#include "PidGuesser.h"
#include <string.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <wordexp.h>
#include <sys/ptrace.h>
#include "Log.h"

PidGuesser::PidGuesser()
{
}

pid_t PidGuesser::WaitForPid(list<string>& cmds) const
{
	pid_t pid;
	do
	{
		pid = FindPid(cmds);
		usleep(50);
	} 
	while(pid == -1);

	return pid;
}

pid_t PidGuesser::StartProcess(string& progToExec) const
{
	wordexp_t result;
	Log log("starter");

	int res = wordexp(progToExec.c_str(), &result, 0);
	if(res != 0) 
	{
		log.Error("Unable to parse command line:", progToExec);
		return -1;
	}
	
	pid_t pid = fork();
	if(pid < 0)
	{
		log.Error("Fork error: ", errno);
		return -1;
	}

	if(pid == 0)
	{
		ptrace(PTRACE_TRACEME, 0, 0, 0);

		log.Info("Starting", progToExec);
	
		int ret = execvp(result.we_wordv[0], result.we_wordv);
		// never reached if exec succeeds, just to avoid warning message
		if(ret!= 0)
		{
			log.Error("Return code of exec:", ret, "errno:", errno);
		}
		return -1;
	}
	else
	{
		wordfree(&result);
		return pid;
	}
}

pid_t PidGuesser::FindPid(list<string>& cmds) const
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

		for(list<string>::const_iterator i = cmds.begin();i!= cmds.end();i++)
		{
			if(procstr == *i)
				return pid;
		}
	}	

	pclose(fp);

	return -1;
}


