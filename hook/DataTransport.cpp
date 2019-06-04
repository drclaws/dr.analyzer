#include "stdafx.h"
#include "DataTransport.h"

#include <exception>
#include <string>

#include "hook_funcs.h"

#include "TransferInfo.h"
#include "BuffObject.h"


DataTransport::DataTransport(int pid)
{
	this->pid = pid;

	// Create connection
	std::wstring mapping_loc = L"dr_analyzer_buffer_" + std::to_wstring(pid);
	std::wstring mutex_name = L"dr_analyzer_mutex_" + std::to_wstring(pid);
	std::wstring semaphore_loc = L"dr_analyzer_semaphore_" + std::to_wstring(pid);

	this->transportMapping = OpenFileMappingW(
		FILE_MAP_WRITE,
		FALSE,
		mapping_loc.c_str()
	);

	if (this->transportMapping == NULL) {
		throw std::exception("Connection error");
	}

	this->transportMutex = OpenMutexW(
		MUTEX_ALL_ACCESS,
		FALSE,
		mutex_name.c_str()
	);

	if (this->transportMutex == NULL) {
		this->CloseConnections();
		throw std::exception("Connection error");
	}

	this->transportSemaphore = OpenSemaphoreW(
		SEMAPHORE_ALL_ACCESS,
		FALSE,
		semaphore_loc.c_str()
	);

	if (this->transportSemaphore == NULL) {
		this->CloseConnections();
		throw std::exception("Connection error");
	}

	TransferInfo *info = new TransferInfo(TransStart);
	BuffObject* buff = new BuffObject();
	buff->AddInfo(info);
	this->SendData(buff);

	// Launch sender's thread
	this->senderThread = new std::thread(this->SenderThreadFunc);
}


DataTransport::~DataTransport()
{
	std::unique_lock<std::mutex> uniqueLock(this->queueOperMutex);
	this->queueOperMutex.lock();
	this->isDisconnecting = true;
	while (this->buffQueue.size() > 0) {
		this->queueOperMutex.unlock();
		this->queueOperEndedCV.wait(this->queueOperMutex);
	}

}


void DataTransport::CloseConnections() {
	if (this->transportMapping != NULL) {
		OrigCloseHandle(this->transportMapping);
		this->transportMapping = NULL;
	}
	if (this->transportMutex != NULL) {
		OrigCloseHandle(this->transportMutex);
		this->transportMutex = NULL;
	}
	if (this->transportSemaphore != NULL) {
		OrigCloseHandle(this->transportSemaphore);
		this->transportSemaphore = NULL;
	}
}
