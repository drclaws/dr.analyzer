// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"

#include "hook.h"
#include "hook_funcs.h"

#include <detours.h>
#include <iostream>

#include <atomic>



DWORD WINAPI ActivateThreadFunc(LPVOID) {
	GatherActivate();
	//return 0;
	ExitThread(0);
}

DWORD WINAPI DeactivateThreadFunc(LPVOID) {
	GatherDeactivate();
	//return 0;
	ExitThread(0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		//std::cout << "injection started" << std::endl;
		if (!GetOrigAddresses()) {
			return FALSE;
		}
		if (CreateThread(NULL, 0, &ActivateThreadFunc, NULL, 0, NULL) == NULL) {
			return FALSE;
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		//std::cout << "Print End" << std::endl;
		if (CreateThread(NULL, 0, &DeactivateThreadFunc, NULL, 0, NULL) == NULL) {
			return FALSE;
		}
		//GatherDeactivate();
		//std::cout << "Print End" << std::endl;
	}
	/*
	if (ul_reason_for_call == DLL_THREAD_DETACH) {
		std::cout << "thread detach" << std::endl;
		if (--encounters == 0) {
			gatherer.Activate();
			return TRUE;
		}
	}*/

    return TRUE;
}
