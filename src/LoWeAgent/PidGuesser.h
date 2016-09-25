#pragma once

#include <list>
#include <string>
#include <stdlib.h>
#include <sys/user.h>
#include <dirent.h>

using namespace std;

class PidGuesser
{
	public:
		PidGuesser(list<string> cmds);
		pid_t GetPid();

	private:
		list<string> _cmds;
		DIR* _dir;

		bool IsNumeric(const char *chars); 
		pid_t Findpid();
};
