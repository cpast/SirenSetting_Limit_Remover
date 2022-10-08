#pragma once
#include "RageStructs.h"
bool ApplyIdHooks(void);
bool ApplyIndexHooks(void);

extern "C" __declspec(dllexport) int EnableRphIntegration(void*);

extern "C" void* SirenSettings_init_patch;
extern "C" void* SirenSettings_init_ret;
extern "C" void* ComputeSirenIndex_patch;
extern "C" void* ComputeSirenIndex_ret;
extern "C" void* ComputeSirenIndex_logic;
extern "C" void* LogConflict_patch;
extern "C" void* LogConflict_nz_ret;
extern "C" void* LogConflict_z_ret;
extern "C" void* LogConflict_logic;
extern "C" void* GetSirenSetting_patch;
extern "C" void* RphOnGetSirenSettings;
extern "C" void* GetSirenSetting_RPH_patch;
extern "C" void* GetSirenSetting_ret;
extern "C" void* CarCols_loc;
extern "C" void* CopyVarToModel_patch;
extern "C" void* CopyVarToModel_ret;
extern "C" void* SetFlags_patch;
extern "C" void* SetFlags_ret;
extern "C" void* SirenSettings_ReallocFree_logic;
extern "C" void* SirenSettings_ReallocFree_patch;
extern "C" void* SirenSettings_ReallocFree_ret;
extern "C" void* SirenSettings_Free_logic;
extern "C" void* SirenSettings_Free_patch;
extern "C" void* SirenSettings_Free_ret;