#include "stdafx.h"
#include "Gatherer.h"

#include <Windows.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <string>

#include <detours.h>

#include "DataTransport.h"


Gatherer* gatherer = NULL;


Gatherer::Gatherer()
{
	this->dataTransport = new DataTransport();

	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	this->queueConnectionThread = new std::thread(this->TransferThreadFunc);

	uniqueLock.unlock();
}

Gatherer::~Gatherer()
{
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);

	uniqueLock.lock();
	this->isDisconnecting = true;
	uniqueLock.unlock();

	this->addCv.notify_one();

	uniqueLock.lock();

	while (this->buffObj != NULL) {
		uniqueLock.unlock();
		std::this_thread::sleep_for(std::chrono::microseconds(10));
		uniqueLock.lock();
	}

	this->queueConnectionThread->join();
	delete this->queueConnectionThread;

	delete this->dataTransport;
	uniqueLock.unlock();
}


void Gatherer::AddFileToBuff(HANDLE fileHandle) {
	//LPCWSTR name;
	//INT32 nameLength;
	// TODO GetFileInfo


	this->AddToBuff(new TransferInfo(TransFileOpen, name, nameLength));
}

void Gatherer::AddLibToBuff(HANDLE libHandle) {
	//LPCWSTR name;
	//INT32 nameLength;
	// TODO GetLibInfo


	this->AddToBuff(new TransferInfo(TransLibraryOpen, name, nameLength));
}

void Gatherer::WarningNameToBig(INT8 fileType)
{
	this->AddToBuff(new TransferInfo(fileType, NULL, (INT32)-1));
}

void Gatherer::AddToBuff(TransferInfo *info) {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	if (this->isDisconnecting) {
		uniqueLock.unlock();
		delete info;
		return;
	}

	if (this->buffObj == NULL) {
		this->buffObj = new BuffObject();
	}

	while (!this->buffObj->AddInfo(info)) {
		this->buffFull = true;
		this->buffFullCv.notify_one();

		uniqueLock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		uniqueLock.lock();
		if (this->buffObj == NULL) {
			this->buffObj = new BuffObject();
		}
	}

	if (!this->threadNotified) {
		this->addCv.notify_one();
	}

	uniqueLock.unlock();
}



void Gatherer::AddLoadedResToBuff() {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	if (this->isDisconnecting) {
		uniqueLock.unlock();
		return;
	}

	// TODO GetCurrentInfo

	uniqueLock.unlock();
}

void Gatherer::TransferThreadFunc() {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	std::unique_lock<std::mutex> addCVLock(this->addCvMutex);
	std::unique_lock<std::mutex> buffFullCVLock(this->buffFullCvMutex);

	std::chrono::high_resolution_clock::time_point waitStart;
	std::chrono::milliseconds duration;

	while (true) {
		waitStart = std::chrono::high_resolution_clock::now();

		// Wait for buff input 
		this->addCv.wait(addCVLock);
		this->threadNotified = true;

		duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - waitStart);

		if (!this->buffFull && duration.count() < 1000ll) {
			// Wait if package was 
			this->buffFullCv.wait_for(buffFullCVLock, std::chrono::milliseconds(1000) - duration);
		}

		uniqueLock.lock();

		if (this->buffObj != NULL) {
			this->dataTransport->SendData(this->buffObj);
		}
		this->buffObj == NULL;
		this->threadNotified = false;
		
		uniqueLock.unlock();

		if (this->isDisconnecting) {
			break;
		}
	}
}
