// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "SirenSettings_patcher.h"
#include "SirenLights.h"
#include "hooking.h"
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
        success = InitializeNearHooks();
        if (!success) {
            log("Page allocation failed!\n");
        }
        else {
            success = ApplyIdHooks();
            if (!success) {
                log("ID hook application failed!\n");
            }
            else {
                log("ID hooks applied.\n");
                success = ApplyIndexHooks();
                if (!success) {
                    log("Index hook application failed!\n");
                }
                else {
                    log("Index hooks applied.\n");
                }
                //MessageBoxA(NULL, "Foo", NULL, MB_OK);
                success = ApplySirenBufferHooks();
                if (!success) {
                    log("Buffer hook application failed!\n");
                }
                else {
                    log("Buffer hooks applied.\n");
                }
            }
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

