#include "FrameBufferTransporter.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/uio.h>

FrameBufferTransporter::FrameBufferTransporter() 
{
	_threadsRunning = false;
}
	
void FrameBufferTransporter::StartThreads(int pid, int size, long remoteAddress, SocketCommunicator *socketCommunicator)
{
	_pid = pid;
	_bufferSize = size;
	_remoteAddress = remoteAddress;
	_socketCommunicator = socketCommunicator;
	_buffer = malloc(size);
	_threadsRunning = true;

	pthread_create(&_readerThreadId, NULL, &ReaderThread_Proc, this);
	pthread_create(&_senderThreadId, NULL, &SenderThread_Proc, this);
}

void FrameBufferTransporter::StopThreads()
{
	if(_threadsRunning)
	{
		_threadsRunning = false;
		pthread_join(_readerThreadId, NULL);
		pthread_join(_senderThreadId, NULL);
		free(_buffer);
	}
}

void *FrameBufferTransporter::ReaderThread_Proc(void *arg)
{
	FrameBufferTransporter *fbTransporter = (FrameBufferTransporter *)arg;
	struct iovec local, remote;

	local.iov_base = fbTransporter->_buffer;
	local.iov_len = fbTransporter->_bufferSize;
	remote.iov_base = (void *)fbTransporter->_remoteAddress;
	remote.iov_len = fbTransporter->_bufferSize;

	while(fbTransporter->_threadsRunning)
	{
		process_vm_readv(fbTransporter->_pid, &local, 1, &remote, 1, 0);
		usleep(1000000/50);
	}

	return NULL;
}

void *FrameBufferTransporter::SenderThread_Proc(void *arg)
{
	FrameBufferTransporter *fbTransporter = (FrameBufferTransporter *)arg;

	while(fbTransporter->_threadsRunning)
	{
		fbTransporter->_socketCommunicator->Send("FRAM", 4);
		fbTransporter->_socketCommunicator->Send((const char *)fbTransporter->_buffer, fbTransporter->_bufferSize);
		usleep(1000000/50);
	}
	return NULL;
}

