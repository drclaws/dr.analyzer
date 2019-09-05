#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>

#include "BuffObject.h"


class DataTransport
{
public:
	DataTransport();
	~DataTransport();

	void SendData(BuffObject *info);

private:
	bool isDisconnecting = false;

	HANDLE transportMapping = NULL;
	PBYTE transportView = NULL;
	HANDLE transportMutex = NULL;
	HANDLE transportSemaphore = NULL;
    HANDLE transportReceivedSemaphore = NULL;
    
	std::thread senderThread;
	std::mutex queueOperMutex;

	std::mutex queueOperEndedMutex;
	std::condition_variable queueOperEndedCV;

	std::queue<BuffObject*> buffQueue;

	void SenderThreadFunc();

	void CloseSharedMemory();
};

