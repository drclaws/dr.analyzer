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

#include "GatherInfo.h"
#include "BuffObject.h"
#include "flags.h"


DataTransport::DataTransport()
{
	// Create connection
	DWORD pid = GetCurrentProcessId();

	std::wstring mapping_loc = std::wstring(L"Global\\") + L"Global\\dr_analyzer_buffer_" + std::to_wstring(pid);
	std::wstring mutex_name = std::wstring(L"Global\\") + L"Global\\dr_analyzer_mutex_" + std::to_wstring(pid);
	std::wstring semaphore_loc = std::wstring(L"Global\\") + L"Global\\dr_analyzer_semaphore_" + std::to_wstring(pid);

	this->transportMapping = OpenFileMappingW(
		FILE_MAP_WRITE,
		FALSE,
		mapping_loc.c_str()
	);

	if (this->transportMapping == NULL) {
		throw std::exception("Connection error: Can't open mapping");
	}

	this->transportView = (INT8*)MapViewOfFile(
		this->transportMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BUFF_MAX_SIZE + sizeof(buff_size_t)
	);

	if (this->transportView == NULL) {
		this->CloseSharedMemory();
		throw std::exception("Connection error: Can't assign view");
	}

	this->transportMutex = OpenMutexW(
		MUTEX_ALL_ACCESS,
		FALSE,
		mutex_name.c_str()
	);

	if (this->transportMutex == NULL) {
		this->CloseSharedMemory();
		throw std::exception("Connection error: Can't open mutex");
	}

	this->transportSemaphore = OpenSemaphoreW(
		SEMAPHORE_ALL_ACCESS,
		FALSE,
		semaphore_loc.c_str()
	);

	if (this->transportSemaphore == NULL) {
		this->CloseSharedMemory();
		throw std::exception("Connection error: Can't open Semaphore");
	}
}


DataTransport::~DataTransport()
{
	if (this->SenderActive) {
		std::unique_lock<std::mutex> uniqueLock(this->queueOperMutex);

		uniqueLock.lock();
		this->isDisconnecting = true;
		BuffObject* buff = new BuffObject();
		buff->AddInfo(new GatherInfo(GatherType::GatherDeactivated, GatherFuncType::GatherConnection));
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
	}

	this->CloseSharedMemory();
}

void DataTransport::SendData(BuffObject* info)
{
	if (!this->SenderActive) {
		return;
	}

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

bool DataTransport::ActivateSender()
{
	if (this->SenderActive) {
		return false;
	}

	BuffObject* buff = new BuffObject();
	buff->AddInfo(new GatherInfo(GatherType::GatherActivated, GatherFuncType::GatherConnection));
	this->SendData(buff);

	// Launch sender's thread
	this->senderThread = new std::thread(&DataTransport::SenderThreadFunc, this);

	this->SenderActive = true;

	return true;
}


void DataTransport::SenderThreadFunc()
{
	if (!this->SenderActive) {
		return;
	}

	std::unique_lock<std::mutex> uniqueLock(this->queueOperMutex);
	std::unique_lock<std::mutex> cvLock(this->queueOperEndedMutex);

	DWORD waitRes;

	while (true) {
		this->queueOperEndedCV.wait_for(cvLock, std::chrono::seconds(5));
		uniqueLock.lock();
		
		if (this->buffQueue.size() == 0) {
			BuffObject* buff = new BuffObject();
			buff->AddInfo(new GatherInfo(GatherType::GatherStillUp, GatherFuncType::GatherConnection));
			this->buffQueue.push(buff);
		}

		while (!this->buffQueue.empty()) {
			BuffObject* buff = this->buffQueue.front();
			uniqueLock.unlock();

			INT8* message = buff->ToMessage();
			
			waitRes = WaitForSingleObject(this->transportMutex, INFINITE);

			std::memcpy(this->transportView, message, buff->MessageSize());

			ReleaseMutex(this->transportMutex);
			ReleaseSemaphore(this->transportSemaphore, 1, NULL);

			// Wait for message read ends
			waitRes = WaitForSingleObject(this->transportSemaphore, 0L);
			
			uniqueLock.lock();
			this->buffQueue.pop();
			delete buff;
			delete[] message;
		}

		if (this->isDisconnecting) {
			uniqueLock.unlock();
			break;
		}

		uniqueLock.unlock();
	}
}

void DataTransport::CloseSharedMemory() {
	if (this->transportView != NULL) {
		UnmapViewOfFile(this->transportView);
		this->transportView = NULL;
	}
	if (this->transportMapping != NULL) {
		CloseHandle(this->transportMapping);
		this->transportMapping = NULL;
	}
	if (this->transportMutex != NULL) {
		CloseHandle(this->transportMutex);
		this->transportMutex = NULL;
	}
	if (this->transportSemaphore != NULL) {
		CloseHandle(this->transportSemaphore);
		this->transportSemaphore = NULL;
	}
}
