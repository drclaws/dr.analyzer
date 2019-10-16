#pragma once

#include "stdafx.h"


// Gets origin addresses of functions for detouring
bool GetOrigAddresses();

bool DetourExitProcess();

void UndetourExitProcess();

bool DetourFuncs();

bool UndetourFuncs();
