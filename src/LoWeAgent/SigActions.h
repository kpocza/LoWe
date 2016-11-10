#pragma once

#include <sys/types.h>

class SigActions
{
	public:
		static void InstallTerminationHandlers(pid_t pid);

	private:
		static pid_t _pid;

		static void InstallTerminationHandler(int signum);
		static void TerminationHandler(int signum);
};


