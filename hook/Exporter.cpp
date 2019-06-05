#include "stdafx.h"
#include "Exporter.h"

#include <Windows.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

#include <detours.h>

#include "DataTransport.h"


Exporter::Exporter(int pid)
{
	this->dataTransport = new DataTransport(pid);

	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	this->queueConnectionThread = new std::thread(this->TransferThreadFunc);

	// TODO Detour

	uniqueLock.unlock();
}

Exporter::~Exporter()
{

}
/*
void Exporter::Disconnect() {
	this->queueMutex->lock();
	this->isDisconnecting = true;

	// TODO ReDetour
	// TODO Add Disconnect with last data
	// TODO Wait Disconnect send

	// TODO Disconnect from mapping and mutex
	this->CloseConnections();

	this->transferPid = 0;
	this->isDisconnecting = false;
	this->isConnected = false;
	this->queueMutex->unlock();
}
*/


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

	TransferInfo *info = new TransferInfo(isOpen ? TransFileOpen : TransFileClose, name, nameLength);
	this->AddToBuff(info);

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

	TransferInfo *info = new TransferInfo(TransLibraryOpen, name, nameLength);
	this->AddToBuff(info);

	uniqueLock.unlock();
}

void Exporter::AddToBuff(TransferInfo *info) {
	// TODO AddToQueue
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
	// TODO TransferThreadFunc
}
