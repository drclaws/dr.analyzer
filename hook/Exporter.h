#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>


#include "BuffObject.h"
#include "DataTransport.h"


static class Exporter
{
public:
	Exporter(int pid);
	~Exporter();

	void AddFileToBuff(HANDLE fileHandle, bool isOpen);
	void AddLibToBuff(HANDLE libHandle);

private:
	bool isDisconnecting = false;
	std::atomic_bool threadNotified = false;

	DataTransport* dataTransport = NULL;

	std::thread *queueConnectionThread = NULL;
	std::mutex buffMutex;

	std::condition_variable addCv;
	std::mutex addCvMutex;

	std::condition_variable buffFullCv;
	std::mutex buffFullCvMutex;
	std::atomic_bool buffFull = false;

	BuffObject* buffObj;

	INT64 queueWaiting = 0;

	void AddToBuff(TransferInfo* info);
	void AddLoadedResToBuff();
	void TransferThreadFunc();
};
