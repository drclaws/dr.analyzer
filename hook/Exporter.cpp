#include "stdafx.h"
#include "Exporter.h"

#include <Windows.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

#include <detours.h>


INT32 TransferInfo::GetSize() {
	const INT32 vals = sizeof(INT8) + sizeof(INT32);
	return nameLength * sizeof(LPCWSTR) + vals;
}


Exporter::Exporter()
{
	this->queueMutex = new std::mutex();
	this->queueCondVar = new std::condition_variable();
}

Exporter::~Exporter()
{
	if (this->isConnected) {
		Disconnect();
	}

	delete this->queueMutex;
	delete this->queueCondVar;
}


bool Exporter::Connect(int pid) {
	this->queueMutex->lock();
	if (this->isConnected) {
		return false;
	}

	this->dataTransport = new DataTransport(pid);

	/*
	this->transferPid = pid;

	// Creates connections
	std::wstring mapping_loc = L"dr_analyzer_buffer_" + std::to_wstring(pid);
	std::wstring mutex_name = L"dr_analyzer_mutex_" + std::to_wstring(pid);
	std::wstring semaphore_loc = L"dr_analyzer_semaphore_" + std::to_wstring(pid);
	this->transferMapping = OpenFileMappingW(
								FILE_MAP_WRITE,
								FALSE,
								mapping_loc.c_str()
							);

	if (this->transferMapping == NULL) {
		this->queueMutex->unlock();
		return false;
	}
	
	this->transferMutex = OpenMutexW(
							  MUTEX_ALL_ACCESS,
							  FALSE,
							  mutex_name.c_str()
						  );

	if (this->transferMutex == NULL) {
		this->CloseConnections();
		this->queueMutex->unlock();
		return false;
	}

	this->transferSemafor = OpenSemaphoreW(
								SEMAPHORE_ALL_ACCESS,
								FALSE,
								semaphore_loc.c_str()
							);

	if (this->transferSemafor == NULL) {
		this->CloseConnections();
		this->queueMutex->unlock();
		return false;
	}

	// Send "Connection set"
	TransferInfo *info = new TransferInfo(TransStart);
	this->AddToBuff(info);*/
	
	// Launch queue thread
	this->queueThread = new std::thread(this->TransferThreadFunc);

	// TODO Detour

	this->isConnected = true;

	this->queueMutex->unlock();
	
	return true;
}

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



void Exporter::AddFileToBuff(HANDLE fileHandle, bool isOpen) {
	this->queueMutex->lock();
	if (!this->isConnected || this->isDisconnecting) {
		this->queueMutex->unlock();
		return;
	}

	LPCWSTR filename;
	INT32 length;
	// TODO GetFileInfo

	TransferInfo *info = new TransferInfo(isOpen ? TransFileOpen : TransFileClose, filename, length);
	this->AddToBuff(info);

	this->queueMutex->unlock();
}

void Exporter::AddLibToBuff(HANDLE libHandle) {
	this->queueMutex->lock();
	if (!this->isConnected || this->isDisconnecting) {
		this->queueMutex->unlock();
		return;
	}

	LPCWSTR filename;
	INT32 length;
	// TODO GetLibInfo

	TransferInfo *info = new TransferInfo(TransLibraryOpen, filename, length);
	this->AddToBuff(info);

	this->queueMutex->unlock();
}

void Exporter::AddToBuff(TransferInfo *info) {
	// TODO AddToQueue
}



void Exporter::GetCurrentInfo() {
	this->queueMutex->lock();
	if (!this->isConnected || this->isDisconnecting) {
		this->queueMutex->unlock();
		return;
	}

	// TODO GetCurrentInfo

	this->queueMutex->unlock();
}

void Exporter::TransferThreadFunc() {
	// TODO TransferThreadFunc
}
