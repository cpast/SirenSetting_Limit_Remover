#pragma once

bool ApplyIdHooks(void);
bool ApplyIndexHooks(void);
extern "C" __declspec(dllexport) bool ApplyRphHook(void);

extern "C" void* SirenSettings_init_patch;
extern "C" void* SirenSettings_init_ret;
extern "C" void* ComputeSirenIndex_patch;
extern "C" void* ComputeSirenIndex_ret;
extern "C" void* ComputeSirenIndex_logic;
extern "C" void* GetSirenSetting_patch;
extern "C" void* GetSirenSetting_ret;
extern "C" void* CarCols_loc;
extern "C" void* CopyVarToModel_patch;
extern "C" void* CopyVarToModel_ret;
extern "C" void* SetFlags_patch;
extern "C" void* SetFlags_ret;
extern "C" void* RphHookPrep_patch;
extern "C" void* RphHookPrep_ret;