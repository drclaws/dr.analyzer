#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "BuffObject.h"


class DataTransport
{
public:
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

	std::mutex queueOperEndedMutex;
	std::condition_variable queueOperEndedCV;

	std::queue<BuffObject*> buffQueue;

	void SenderThreadFunc();

	void CloseConnections();
};

