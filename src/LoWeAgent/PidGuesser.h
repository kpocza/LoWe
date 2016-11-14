#pragma once

#include <list>
#include <string>

using namespace std;

class PidGuesser
{
	public:
		PidGuesser();
		pid_t WaitForPid(list<string>& cmds) const;
		pid_t StartProcess(string& progToExec) const;
		static string GetProgramName(string &progToExec);

	private:
		pid_t FindPid(list<string>& cmds) const;
};
