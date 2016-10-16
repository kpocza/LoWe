#include "SocketCommunicator.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "Log.h"

SocketCommunicator::SocketCommunicator()
{
	_opened = false;
}

bool SocketCommunicator::Open(const string address, const int port)
{
	if(_opened)
		return true;
	Log log("socket");

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(_fd < 0)
	{
		log.Debug("Socket open errno:", errno);
		return false;
	}
	
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(address.c_str());

	if(connect(_fd, (const sockaddr *)&addr, sizeof(addr)) < 0)
	{
		log.Debug("Socket connect errno:", errno);
		return false;
	}
	_opened = true;

	return true;
}

bool SocketCommunicator::Send(const char *buffer, int len)
{
	if(!_opened)
		return false;

	int addr = 0;

	while(len > 0)
	{
		int sent = send(_fd, buffer + addr, len, 0);
		if(sent < 1)
			return false;
		addr+=sent;
		len-=sent;
	}
	return true;
}

bool SocketCommunicator::Recv(char *buffer, int len)
{
	if(!_opened)
		return false;

	int addr = 0;

	while(len > 0)
	{
		int recvd = recv(_fd, buffer + addr, len, 0);
		if(recvd < 1)
			return false;
		addr+=recvd;
		len-=recvd;
	}
	return true;
}

void SocketCommunicator::Close()
{
	if(_opened)
	{
		close(_fd);
		_opened = false;
	}
}
