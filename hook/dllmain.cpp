// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"

#include "hook.h"
#include "hook_funcs.h"

#include <detours.h>


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		if (GetOrigAddresses() == TRUE) {
			return StartGathering();
		}
		else {
			return FALSE;
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		return StopGathering();
	}
    return TRUE;
}

