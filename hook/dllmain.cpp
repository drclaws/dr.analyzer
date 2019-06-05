// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"

#include "hook.h"
#include "hook_funcs.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		if (GetOrigAddresses() == TRUE) {
			return StartGathering();
		}
		else {
			return FALSE;
		}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		return StopGathering();
    }
    return TRUE;
}

