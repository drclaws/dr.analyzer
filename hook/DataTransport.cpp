#include "stdafx.h"
#include "DataTransport.h"

#include <Windows.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

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

	BuffObject* buff = new BuffObject();
	buff->AddInfo(new TransferInfo(TransOpen));
	this->SendData(buff);

	// Launch sender's thread
	this->senderThread = new std::thread(this->SenderThreadFunc);
}


DataTransport::~DataTransport()
{
	std::unique_lock<std::mutex> uniqueLock(this->queueOperMutex);

	uniqueLock.lock();
	this->isDisconnecting = true;
	BuffObject* buff = new BuffObject();
	buff->AddInfo(new TransferInfo(TransClose));
	uniqueLock.unlock();

	this->SendData(buff);

	uniqueLock.lock();
	while (this->buffQueue.size() > 0) {
		uniqueLock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		uniqueLock.lock();
	}
	uniqueLock.unlock();

	this->senderThread->join();
	delete this->senderThread;

	this->CloseConnections();
}

void DataTransport::SendData(BuffObject* info)
{
	std::unique_lock<std::mutex> uniqueLock(this->queueOperMutex);

	uniqueLock.lock();
	if (this->isDisconnecting) {
		uniqueLock.unlock();
		return;
	}

	this->buffQueue.push(info);
	this->queueOperEndedCV.notify_one();

	uniqueLock.unlock();
}


void DataTransport::SenderThreadFunc()
{
	std::unique_lock<std::mutex> uniqueLock(this->queueOperMutex);
	std::unique_lock<std::mutex> cvLock(this->queueOperEndedMutex);

	DWORD waitRes;

	while (true) {
		this->queueOperEndedCV.wait(cvLock, std::chrono::seconds(5));
		uniqueLock.lock();
		
		if (this->buffQueue.size() == 0) {
			BuffObject* buff = new BuffObject();
			buff->AddInfo(new TransferInfo(TransStillUp));
			this->buffQueue.push(buff);
		}

		while (!this->buffQueue.empty()) {
			BuffObject* buff = this->buffQueue.front();
			
			// TODO Convert to message
			
			waitRes = WaitForSingleObject(this->transportMutex, INFINITE);
			// TODO check status

			// TODO Write to memory

			ReleaseMutex(this->transportMutex);

			ReleaseSemaphore(this->transportSemaphore, 1, NULL);
			waitRes = WaitForSingleObject(this->transportSemaphore, 0L);
			// TODO check status'

			this->buffQueue.pop();
			delete buff;
		}

		uniqueLock.unlock();

		if (this->isDisconnecting) {
			break;
		}
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
