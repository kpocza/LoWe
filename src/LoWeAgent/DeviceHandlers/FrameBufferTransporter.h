#pragma once

#include <SocketCommunicator.h>
#include <pthread.h>

class FrameBufferTransporter 
{
	public:
		FrameBufferTransporter(); 
	
		void StartThreads(int pid, int size, long remoteAddress, SocketCommunicator *socketCommunicator);
		void StopThreads();

	private:
		static void *ReaderThread_Proc(void *arg);
		static void *SenderThread_Proc(void *arg);

		int _pid;
		int _bufferSize;
		long _remoteAddress;
		SocketCommunicator *_socketCommunicator;

		volatile bool _threadsRunning;
		void *_buffer;

		pthread_t _readerThreadId;
		pthread_t _senderThreadId;
};

