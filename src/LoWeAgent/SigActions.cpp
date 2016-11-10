#include "SigActions.h"
#include <signal.h>

pid_t SigActions::_pid = -1;

void SigActions::InstallTerminationHandlers(pid_t pid)
{
	if(pid < 0)
		return;

	SigActions::_pid = pid;

	InstallTerminationHandler(SIGTERM);
	InstallTerminationHandler(SIGINT);
	InstallTerminationHandler(SIGHUP);
	InstallTerminationHandler(SIGQUIT);
}

void SigActions::InstallTerminationHandler(int signum)
{
	struct sigaction newAction, oldAction;

	newAction.sa_handler = SigActions::TerminationHandler;
	sigemptyset(&newAction.sa_mask);
	newAction.sa_flags = 0;

	sigaction(signum, 0, &oldAction);
	if(oldAction.sa_handler != SIG_IGN)
		sigaction(signum, &newAction, 0);
}

void SigActions::TerminationHandler(int signum)
{
	kill(SigActions::_pid, SIGTERM);
}


