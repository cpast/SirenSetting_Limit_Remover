#pragma once
#include <map>
#include <cstdint>

extern "C" void* GetSirenBoneArray(void* vehicle);
extern "C" int32_t * BoneArray;
extern std::map<uint32_t, int32_t *> BoneMap;