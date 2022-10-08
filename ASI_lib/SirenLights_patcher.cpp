#include "pch.h"
#include "SirenSettings_patcher.h"
#include "SirenLights.h"
#include "RageStructs.h"
#include "hooking.h"
#include "debug.h"
#include <cstdlib>
#include <cassert>

CRITICAL_SECTION mapMutex;

std::map<uint32_t, int32_t*> BoneMap;

int32_t (*GetBoneIndexFromId)(void* entity, uint16_t boneTag) = NULL;
CSirenSettingsExpanded expandedSettings = { 0 };

const Pattern DrawSirenLights_Pattern = { "0f 29 70 a8 0f 29 78 98 44 0f 29 40 88 4c 8b f9 44 0f 29 88 78 ff ff ff 44 0f 29 90 68 ff ff ff 44 0f 29 98 58 ff ff ff", 0x27 };
const Pattern GetHeadlightStatus_Pattern = { "0f b6 d2 41 0f b6 c0 4c 8b c9 8d 0c 50 0f b6 d1 41 8b 89 84 01 00 00", 0 };
const Pattern GetHeadlightIntensity_Pattern = { "0f b6 d2 41 0f b6 c0 8d 14 50 f3 0f 10 84 91 6c 01 00 00 c3", 0 };
const Pattern CheckForBrokenSirens_Pattern = { "4c 8b 8f ?? ?? ?? ?? 49 8b 41 20 48 8b 88 b0 00 00 00 48 8b 01 0f be 54 30 10", 0x68e };

int32_t CheckSirenBoneArray(uint32_t bone, CVehicle* vehicle)
{
	uint32_t vehName = vehicle->modelInfo->nameHash;
	assert(BoneMap.count(vehName));
	return BoneMap.at(vehName)[bone];
}

void MakeSirenBoneArray(CVehicle* vehicle)
{
	uint32_t vehName = vehicle->modelInfo->nameHash;
	EnterCriticalSection(&mapMutex);
	logDebug("%8.8x: ", vehName);
	if (BoneMap.count(vehName))
	{
		logDebug("FOUND %p\n", BoneMap.at(vehName));
	}
	else
	{
		logDebug("Building... ");
		int32_t* newArray = (int32_t*)malloc(NUM_LIGHTS * sizeof(int32_t));
		if (newArray == NULL)
		{
			LeaveCriticalSection(&mapMutex);
			return;
		}
		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			newArray[i] = GetBoneIndexFromId((void*)vehicle, BoneTags[i]);
		}
		BoneMap[vehName] = newArray;
		logDebug("Built: %p\n", newArray);
	}
	LeaveCriticalSection(&mapMutex);
	return;
}

void InitializeSirenBufferBesidesSeed(SirenBuffer* buffer)
{
	buffer->BeatNumber = -1;
	buffer->AdjustedTime = 0;
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		buffer->FlasherLastBeatMask[i] = 0;
		buffer->FlasherLastBeatTime[i] = 0;
		buffer->RotatorLastBeatMask[i] = 0;
		buffer->RotatorLastBeatTime[i] = 0;
	}
	for (int i = 0; i < 4; i++)
	{
		buffer->HeadlightLastBeatMask[i] = 0;
		buffer->HeadlightLastBeatTime[i] = 0;
		buffer->HeadlightIntensity[i] = 1.0f;
	}
	buffer->RotatorStatus = 0;
	buffer->FlasherStatus = 0;
	buffer->HeadlightStatus = 0;
}

bool ApplySirenBufferHooks()
{
	InitializeCriticalSection(&mapMutex);
	pExpandedSettings = &expandedSettings;
	BoneSetup_logic = (uintptr_t) &MakeSirenBoneArray;
	BoneCheck_logic = (uintptr_t) &CheckSirenBoneArray;
	ExpandSettings_logic = (uintptr_t)&ExpandSirenSettings;
	free_logic = (uintptr_t)&free;
	pattern BufferMallocPtn = { "48 8b d8 48 85 c0 74 0c 33 d2 48 8b c8", 0x5b5 };
	uintptr_t BufferMallocLoc = FindPattern(BufferMallocPtn);
	if (BufferMallocLoc == NULL)
	{
		return false;
	}
	uint32_t BufferSize = 0x2d0;
	WriteForeignMemory(BufferMallocLoc + 0x5ac, &BufferSize, sizeof(uint32_t));
	uintptr_t BufferInitLoc = GetReferencedAddress(BufferMallocLoc + 0x5c2);
	if (BufferInitLoc == NULL)
		return false;
	if (InsertHookWithSkip(BufferInitLoc + 0x1c, BufferInitLoc + 0x2d, (uintptr_t)&InitializeSirenBufferBesidesSeed) == NULL)
	{
		return false;
	}
	pattern BoneIndexPtn = { "e8 ?? ?? ?? ?? 8b d0 83 f8 ff 74 17 48 8b 07 4c 8d 44 24", 0x61 };
	uintptr_t BoneIndexLoc = FindPattern(BoneIndexPtn);
	if (BoneIndexLoc == NULL)
	{
		return false;
	}
	BoneIndexLoc = GetReferencedAddress(BoneIndexLoc + 0x61);
	if (BoneIndexLoc == NULL)
	{
		return false;
	}
	GetBoneIndexFromId = (int32_t(*)(void*, uint16_t))BoneIndexLoc;

	uintptr_t CheckBrokenLoc = FindPattern(CheckForBrokenSirens_Pattern);
	if (CheckBrokenLoc == NULL)
		return false;
	CheckBroken_ret = InsertHookWithSkip(CheckBrokenLoc + 0x695, CheckBrokenLoc + 0x6a8, (uintptr_t)&CheckBroken_patch);
	if (CheckBroken_ret == NULL)
		return false;
	int32_t endBoneId = *(int32_t*)(CheckBrokenLoc + 0x7bc) + 12;
	WriteForeignMemory(CheckBrokenLoc + 0x7bc, &endBoneId, 4);

	uintptr_t DrawSirenLights = FindPattern(DrawSirenLights_Pattern);
	if (DrawSirenLights == NULL)
		return false;

	uintptr_t GetHeadlightStatus = FindPattern(GetHeadlightStatus_Pattern);
	uintptr_t GetHeadlightIntensity = FindPattern(GetHeadlightIntensity_Pattern);

	if (!GetHeadlightIntensity || !GetHeadlightStatus)
		return false;

	bool success = true;
	
	{
		uint32_t RotatorBeatMask = offsetof(SirenBuffer, RotatorLastBeatMask);
		uint32_t RotatorBeatTime = offsetof(SirenBuffer, RotatorLastBeatTime);
		uint32_t FlasherBeatMask = offsetof(SirenBuffer, FlasherLastBeatMask);
		uint32_t FlasherBeatTime = offsetof(SirenBuffer, FlasherLastBeatTime);
		uint32_t LightBeatMask = offsetof(SirenBuffer, HeadlightLastBeatMask);
		uint32_t LightBeatTime = offsetof(SirenBuffer, HeadlightLastBeatTime);
		uint32_t LightIntensity = offsetof(SirenBuffer, HeadlightIntensity);
		uint32_t LightStatus = offsetof(SirenBuffer, HeadlightStatus);
		uint32_t FlasherStatus = offsetof(SirenBuffer, FlasherStatus);
		uint32_t RotatorStatus = offsetof(SirenBuffer, RotatorStatus);
		uint8_t SirenCount = 0x20; // TODO: Make 0x28
		uint32_t MaxBoneId = 0;
		uintptr_t FlasherBeatMaskRefs[3] = { 0x5be, 0x5d3, 0x5dd };
		uintptr_t FlasherBeatTimeRefs[2] = { 0x5ec, 0x610 };
		uintptr_t FlasherStatusRefs[6] = { 0x5a9, 0x728, 0x736, 0x80f, 0x884, 0xa03 };
		uintptr_t RotatorStatusRefs[7] = { 0x378, 0x4ad, 0x4ba, 0x538, 0x578, 0x73f, 0x9f9 };
		uintptr_t LightOffsetRefs[1] = { 0x196 };
		uintptr_t LightStatusRefs[3] = { 0x1f0, 0x26e, 0x278 };
		uintptr_t SirenCountRefs[4] = { 0x424, 0x481, 0x66a, 0x6f5 };

		for (int i = 0; i < 3; i++)
			success = success && WriteForeignMemory(DrawSirenLights + FlasherBeatMaskRefs[i], &FlasherBeatMask, sizeof(uint32_t));
		for (int i = 0; i < 2; i++)
			success = success && WriteForeignMemory(DrawSirenLights + FlasherBeatTimeRefs[i], &FlasherBeatTime, sizeof(uint32_t));
		for (int i = 0; i < 6; i++)
			success = success && WriteForeignMemory(DrawSirenLights + FlasherStatusRefs[i], &FlasherStatus, sizeof(uint32_t));
		for (int i = 0; i < 7; i++)
			success = success && WriteForeignMemory(DrawSirenLights + RotatorStatusRefs[i], &RotatorStatus, sizeof(uint32_t));
		for (int i = 0; i < 1; i++)
			success = success && WriteForeignMemory(DrawSirenLights + LightOffsetRefs[i], &LightBeatMask, sizeof(uint32_t));
		for (int i = 0; i < 3; i++)
			success = success && WriteForeignMemory(DrawSirenLights + LightStatusRefs[i], &LightStatus, sizeof(uint32_t));
		for (int i = 0; i < 4; i++)
			success = success && WriteForeignMemory(DrawSirenLights + SirenCountRefs[i], &SirenCount, sizeof(uint8_t));

		DSL_RotateBeatTimeSet_ret = InsertNearHookWithSkip(DrawSirenLights + 0x3ae, DrawSirenLights + 0x3b3, (uintptr_t)&DSL_RotateBeatTimeSet_patch);
		DSL_RotateBeatTimeSub_ret = InsertNearHookWithSkip(DrawSirenLights + 0x403, DrawSirenLights + 0x408, (uintptr_t)&DSL_RotateBeatTimeSub_patch);
		DSL_BoneCheck_ret = InsertHookWithSkip(DrawSirenLights + 0x31b, DrawSirenLights + 0x332, (uintptr_t)&DSL_BoneCheck_patch);
		DSL_PreCompute_ret = InsertNearHook(DrawSirenLights + 0x91, (uintptr_t)&DSL_PreCompute_patch);
		DSL_Return_ret = InsertNearHook(DrawSirenLights + 0x108d, (uintptr_t)&DSL_Return_patch);
		success = success && DSL_RotateBeatTimeSet_ret && DSL_RotateBeatTimeSub_ret && DSL_BoneCheck_ret && DSL_PreCompute_ret && DSL_Return_ret;

		MaxBoneId = *(uint32_t*)(DrawSirenLights + 0xe43);
		MaxBoneId += 12; // TODO: Make 20
		success = success && WriteForeignMemory(DrawSirenLights + 0xe43, &MaxBoneId, sizeof(uint32_t));
	}
	if (!success)
		return false;
	uint32_t LightIntensity = offsetof(SirenBuffer, HeadlightIntensity);
	uint32_t LightStatus = offsetof(SirenBuffer, HeadlightStatus);
	if (!WriteForeignMemory(GetHeadlightIntensity + 0xf, &LightIntensity, sizeof(uint32_t)))
		return false;
	if (!WriteForeignMemory(GetHeadlightStatus + 0x13, &LightStatus, sizeof(uint32_t)))
		return false;

	return true;
}

CSirenSettingsExpanded* ExpandSirenSettings(CSirenSettings* src)
{
	CSirenSettingsExpanded* dst = (CSirenSettingsExpanded*) malloc(sizeof(CSirenSettingsExpanded));
	if (dst == NULL)
		return NULL;
	memcpy(dst, src, 0x3c);
	dst->Name = src->Name;
	dst->NumSirens = src->NumSirens;
	int16_t srcLightNum = src->lightSize;
	if (srcLightNum > NUM_LIGHTS)
		srcLightNum = NUM_LIGHTS;
	memcpy(dst->Lights, src->Lights, srcLightNum * sizeof(EmergencyLight));
	memset(dst->Lights + srcLightNum, NULL, (NUM_LIGHTS - srcLightNum) * sizeof(EmergencyLight));
	return dst;
}