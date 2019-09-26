#pragma once
#include "stdafx.h"

#include "BuffObject.hpp"


extern HMODULE libHModule;

extern HANDLE waiterThread;
extern HANDLE waiterSemaphore;

extern HANDLE freeLibSemaphore;

void SearchFileHandles(BuffObject * &currBuff);

void GetFeaturesSupport();
