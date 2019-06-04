#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "BuffObject.h"


#define MAX_BUFF 76000


class DataTransport
{
public:
	const int maxLength = MAX_BUFF;

	DataTransport(int pid);
	~DataTransport();

	void SendData(BuffObject *info);

private:
	int pid;
	
	bool isDisconnecting = false;

	HANDLE transportMapping = NULL;
	HANDLE transportMutex = NULL;
	HANDLE transportSemaphore = NULL;
	
	std::thread *senderThread = NULL;
	std::mutex queueOperMutex;
	std::condition_variable queueOperEndedCV;

	std::queue<BuffObject*> buffQueue;

	std::thread *senderThread = NULL;

	void SenderThreadFunc();

	void CloseConnections();
};

