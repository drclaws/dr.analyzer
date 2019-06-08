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
	bool ActivateSender();

private:
	std::atomic_bool SenderActive = false;
	bool isDisconnecting = false;

	HANDLE transportMapping = NULL;
	INT8* transportView = NULL;
	HANDLE transportMutex = NULL;
	HANDLE transportSemaphore = NULL;

	std::thread *senderThread = NULL;
	std::mutex queueOperMutex;

	std::mutex queueOperEndedMutex;
	std::condition_variable queueOperEndedCV;

	std::queue<BuffObject*> buffQueue;

	void SenderThreadFunc();

	void CloseSharedMemory();
};

