#include "SirenSettings_patcher.h"
#include "SirenLights.h"
#include "RageStructs.h"
#include "hooking.h"
#include <cstdlib>

std::map<uint32_t, int32_t*> BoneMap;

CSirenSettings_Revised 

int32_t (*GetBoneIndexFromId)(void* entity, uint16_t boneTag) = NULL;

void* GetSirenBoneArray(CVehicle* vehicle)
{
	uint32_t vehName = vehicle->modelInfo->nameHash;
	if (BoneMap.count(vehName))
	{
		BoneArray = BoneMap[vehName];
	}
	else
	{
		int32_t* newArray = (int32_t*)malloc(40 * sizeof(int32_t));
		for (int i = 0; i < 40; i++)
		{
			newArray[i] = GetBoneIndexFromId((void*)vehicle, BoneTags[i]);
		}
		BoneMap[vehName] = newArray;
		BoneArray = newArray;
	}
	return BoneArray;
}

bool ApplySirenBufferHooks()
{
	pattern BufferMallocPtn = { "48 8b d8 48 85 c0 74 0c 33 d2 48 8b c8", 0x5b5 };
	uintptr_t BufferMallocLoc = FindPattern(BufferMallocPtn);
	if (BufferMallocLoc == NULL)
	{
		return false;
	}
	uint32_t BufferSize = 0x2c8;
	WriteForeignMemory(BufferMallocLoc + 0x5ac, &BufferSize, sizeof(uint32_t));
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

}