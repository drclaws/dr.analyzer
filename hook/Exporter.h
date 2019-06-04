#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "TransferInfo.h"
#include "DataTransport.h"


#define MAX_SIZE 500
//#define MAX_BUFF 40000


static class Exporter
{
public:
	Exporter();
	~Exporter();
	
	bool Connect(int pid);
	void Disconnect();
	void AddFileToBuff(HANDLE fileHandle, bool isOpen);
	void AddLibToBuff(HANDLE libHandle);

private:
	bool isConnected = false;
	bool isDisconnecting = false;
//	HANDLE transferMapping = NULL;
//	HANDLE transferMutex = NULL;
//	HANDLE transferSemafor = NULL;
//	int transferPid = 0;

	DataTransport* dataTransport = NULL;

	std::thread *queueThread = NULL;
	std::mutex *queueMutex = NULL;
	std::condition_variable *queueCondVar = NULL;

	INT32 length = 0;
	//const int maxLength = MAX_BUFF;
	int maxLength = MAX_BUFF;
	int currElems = 0;
	const int maxElems = MAX_SIZE;
	TransferInfo *infoBuff[MAX_SIZE];

	INT64 queueWaiting = 0;

	void AddToBuff(TransferInfo* info);
	void GetCurrentInfo();
	void TransferThreadFunc();
	//void CloseConnections();
};
