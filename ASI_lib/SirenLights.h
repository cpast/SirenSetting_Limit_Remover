#pragma once
#include <map>
#include <cstdint>
#include "RageStructs.h"

extern "C" void MakeSirenBoneArray(CVehicle* vehicle);
extern "C" int32_t CheckSirenBoneArray(uint32_t bone, CVehicle * vehicle);
extern "C" CSirenSettingsExpanded * pExpandedSettings;

extern std::map<uint32_t, int32_t *> BoneMap;
extern CSirenSettingsExpanded expandedSettings;
CSirenSettingsExpanded* ExpandSirenSettings(CSirenSettings* src);
bool ApplySirenBufferHooks();

extern "C" uintptr_t DSL_RotateBeatTimeSet_ret;
extern "C" uintptr_t DSL_RotateBeatTimeSub_ret;
extern "C" uintptr_t DSL_PreCompute_ret;
extern "C" uintptr_t BoneSetup_logic;
extern "C" uintptr_t BoneCheck_logic;
extern "C" uintptr_t ExpandSettings_logic;
extern "C" uintptr_t free_logic;
extern "C" uintptr_t DSL_BoneCheck_ret;
extern "C" uintptr_t DSL_Return_ret;
extern "C" uintptr_t CheckBroken_ret;
extern "C" void* DSL_PreCompute_patch;
extern "C" void* DSL_BoneCheck_patch;
extern "C" void* DSL_RotateBeatTimeSet_patch;
extern "C" void* DSL_RotateBeatTimeSub_patch;
extern "C" void* DSL_Return_patch;
extern "C" void* CheckBroken_patch;