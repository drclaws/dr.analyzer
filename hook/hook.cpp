// hook.cpp : Определяет экспортированные функции для приложения DLL.
//
#include "stdafx.h"

#include "hook.h"

int a = 0;

HOOK_API void SetupConnection(int pid) {
	a += pid;
}

