#include "stdafx.h"
#include "Exporter.h"

#include <Windows.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <string>

#include <detours.h>

#include "DataTransport.h"


Exporter::Exporter(int pid)
{
	this->dataTransport = new DataTransport(pid);

	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	this->queueConnectionThread = new std::thread(this->TransferThreadFunc);

	uniqueLock.unlock();
}

Exporter::~Exporter()
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


void Exporter::AddFileToBuff(HANDLE fileHandle, bool isOpen) {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	if (this->isDisconnecting) {
		uniqueLock.unlock();
		return;
	}

	LPCWSTR name;
	INT32 nameLength;
	// TODO GetFileInfo

	this->AddToBuff(new TransferInfo(isOpen ? TransFileOpen : TransFileClose, name, nameLength));

	uniqueLock.unlock();
}

void Exporter::AddLibToBuff(HANDLE libHandle) {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();
	if (this->isDisconnecting) {
		uniqueLock.unlock();
		return;
	}

	LPCWSTR name;
	INT32 nameLength;
	// TODO GetLibInfo

	this->AddToBuff(new TransferInfo(TransLibraryOpen, name, nameLength));

	uniqueLock.unlock();
}

void Exporter::AddToBuff(TransferInfo *info) {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

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



void Exporter::AddLoadedResToBuff() {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	if (this->isDisconnecting) {
		uniqueLock.unlock();
		return;
	}

	// TODO GetCurrentInfo

	uniqueLock.unlock();
}

void Exporter::TransferThreadFunc() {
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
