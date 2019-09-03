#pragma once
#include "stdafx.h"

#include "BuffObject.h"
#include "DataTransport.h"
#include "flags.h"


extern HMODULE libHModule;

extern HANDLE waiterThread;
extern HANDLE waiterSemaphore;

extern HANDLE freeLibSemaphore;

void SearchFileHandles(DataTransport *dataTransport, BuffObject **currBuff);

void GetFeaturesSupport();
