#pragma once

#if defined (_MSC_VER)
#if defined (HOOK_EXPORTS)
#define HOOK_API __declspec(dllexport)
#else
#define HOOK_API __declspec(dllimport)
#endif
#else
#define MY_EXPORT
#endif

int pid;

#ifdef __cplusplus
extern "C" {
#endif

HOOK_API void SetupConnection(int);

#ifdef __cplusplus
}
#endif
