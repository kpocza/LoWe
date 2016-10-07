#pragma once

#include <list>
#include <string>

using namespace std;

class PidGuesser
{
	public:
		PidGuesser(list<string> cmds);
		pid_t GetPid();

	private:
		list<string> _cmds;

		pid_t Findpid();
};
