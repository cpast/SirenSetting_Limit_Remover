#pragma once
#include <map>
#include <cstdint>
#include "RageStructs.h"



extern std::map<uint32_t, int32_t *> SirenBoneMap;
extern CSirenSettingsExpanded expandedSettings;
extern "C" CSirenSettingsExpanded * ExpandSirenSettings(CSirenSettings * src);
extern "C" void MakeBoneArrays(CVehicle * vehicle);
extern "C" int32_t CheckSirenBoneArray(uint32_t bone, CVehicle * vehicle);
extern "C" int32_t CheckGlassBoneArray(uint32_t bone, CVehicle * vehicle);
bool ApplySirenBufferHooks();

extern "C" uintptr_t DSL_RotateBeatTimeSet_ret;
extern "C" uintptr_t DSL_RotateBeatTimeSub_ret;
extern "C" uintptr_t DSL_PreCompute_ret;
extern "C" uintptr_t free_logic;
extern "C" uintptr_t DSL_BoneCheck_ret;
extern "C" uintptr_t DSL_Return_ret;
extern "C" uintptr_t CheckBroken_ret;
extern "C" uintptr_t CheckBrokenTwo_ret;
extern "C" uintptr_t CheckBrokenGlass_ret;
extern "C" uintptr_t CheckBrokenTwoGlass_ret;
extern "C" uintptr_t InitThingy_ret;
extern "C" void* DSL_PreCompute_patch;
extern "C" void* DSL_BoneCheck_patch;
extern "C" void* DSL_RotateBeatTimeSet_patch;
extern "C" void* DSL_RotateBeatTimeSub_patch;
extern "C" void* DSL_Return_patch;
extern "C" void* CheckBroken_patch;
extern "C" void* CheckBrokenTwo_patch;
extern "C" void* CheckBrokenGlass_patch;
extern "C" void* CheckBrokenTwoGlass_patch;
extern "C" void* InitThingy_patch;