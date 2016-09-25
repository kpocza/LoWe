#pragma once

#include <string>

using namespace std;

class SocketCommunicator
{
	public:
		SocketCommunicator();

		bool Open(const string address, const int port);
		bool Send(const char *buffer, int len);
		bool Recv(char *buffer, int len);
		void Close();

	private:
		int _fd;
		bool _opened;
};

