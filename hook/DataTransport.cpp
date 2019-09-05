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
#include "hook.h"

#include <iostream>


DataTransport::DataTransport()
{
	// Create connection
	DWORD pid = GetCurrentProcessId();
    
    std::wstring pidStr = std::to_wstring(pid);
    
	std::wstring mapping_loc = L"Global\\dr_analyzer_buffer_" + pidStr;
	std::wstring mutex_name = L"Global\\dr_analyzer_mutex_" + pidStr;
	std::wstring semaphore_sent_loc = L"Global\\dr_analyzer_sent_semaphore_" + pidStr;
    std::wstring semaphore_received_loc = L"Global\\dr_analyzer_received_semaphore_" + pidStr;
    
	this->transportMapping = OpenFileMappingW(
		FILE_MAP_WRITE,
		FALSE,
		mapping_loc.c_str()
	);

	if (this->transportMapping == NULL) {
		throw std::exception("Connection error: Can't open mapping");
	}

	this->transportView = (PBYTE)MapViewOfFile(
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
		semaphore_sent_loc.c_str()
	);
    
	if (this->transportSemaphore == NULL) {
		this->CloseSharedMemory();
		throw std::exception("Connection error: Can't open Semaphore");
	}
    
    this->transportReceivedSemaphore = OpenSemaphoreW(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        semaphore_received_loc.c_str()
    );
    
    if (this->transportReceivedSemaphore == NULL) {
        this->CloseSharedMemory();
        throw std::exception("Connection error: Can't open Received Semaphore");
    }
    
	BuffObject* buff = new BuffObject();
	buff->AddInfo(new GatherInfo(GatherType::GatherStarted, GatherFuncType::GatherConnection));
	this->SendData(buff);

	// Launch sender's thread
	this->senderThread = std::thread(&DataTransport::SenderThreadFunc, this);
}


DataTransport::~DataTransport()
{
	this->queueOperMutex.lock();

	this->isDisconnecting = true;
	BuffObject* buff = new BuffObject();
	buff->AddInfo(new GatherInfo(GatherType::GatherStopped, GatherFuncType::GatherConnection));
	this->buffQueue.push(buff);
	this->queueOperEndedCV.notify_one();

	this->queueOperMutex.unlock();
	
	this->senderThread.join();

	this->CloseSharedMemory();
}

void DataTransport::SendData(BuffObject* info)
{
	std::unique_lock<std::mutex> operationLock(this->queueOperMutex);
	if (this->isDisconnecting) {
		return;
	}

	this->buffQueue.push(info);
	this->queueOperEndedCV.notify_one();
}


void DataTransport::SenderThreadFunc()
{
	std::unique_lock<std::mutex> cvLock(this->queueOperEndedMutex);

	DWORD waitRes = WAIT_OBJECT_0;

	while (true) {
		this->queueOperEndedCV.wait_for(cvLock, std::chrono::seconds(5));
		this->queueOperMutex.lock();
		
		if (this->buffQueue.size() == 0) {
			BuffObject* buff = new BuffObject();
			buff->AddInfo(new GatherInfo(GatherType::GatherStillUp, GatherFuncType::GatherConnection));
			this->buffQueue.push(buff);
		}

		while (!this->buffQueue.empty()) {
			BuffObject* buff = this->buffQueue.front();

			this->queueOperMutex.unlock();

			PBYTE message = buff->ToMessage();

			if((waitRes = WaitForSingleObject(this->transportMutex, 10000)) != WAIT_OBJECT_0)
			{
			    this->isDisconnecting = true;
			    delete buff;
			    delete[] message;
			    break;
			}

			std::memcpy(this->transportView, message, buff->MessageSize());

			ReleaseMutex(this->transportMutex);
			ReleaseSemaphore(this->transportSemaphore, 1, NULL);
			
			if((waitRes = WaitForSingleObject(this->transportReceivedSemaphore, 10000)) != WAIT_OBJECT_0) {
			    this->isDisconnecting = true;
			    delete buff;
			    delete[] message;
			    break;
			}
			
			this->queueOperMutex.lock();
			this->buffQueue.pop();
			delete buff;
			delete[] message;
		}

		if (this->isDisconnecting) {
			this->queueOperMutex.unlock();
			break;
		}
        
		this->queueOperMutex.unlock();
	}
	
	if(waitRes != WAIT_OBJECT_0) {
	    ReleaseSemaphore(waiterSemaphore, 1, NULL);
	    while(!this->buffQueue.empty()) {
	        delete this->buffQueue.front();
	        this->buffQueue.pop();
	    }
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
	if (this->transportReceivedSemaphore != NULL) {
	    CloseHandle(this->transportReceivedSemaphore);
	    this->transportReceivedSemaphore = NULL;
	} 
}
