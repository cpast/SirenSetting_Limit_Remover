// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <cstdio>
#include "SirenSettings_patcher.h"
#include "debug.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    bool success = true;
    switch (ul_reason_for_call)
    {

    case DLL_PROCESS_ATTACH:
        success = ApplyIdHooks();
        //cleanup_log();
        if (!success) {
            MessageBoxA(NULL, "fail_id", "fail", MB_OK);
        }
        success = ApplyIndexHooks();
        //cleanup_log();
        if (!success) {
            MessageBoxA(NULL, "fail_index", "fail", MB_OK);
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

