#include "pch.h"
#include "RageStructs.h"
#include "SirenSettings_patcher.h"
#include "debug.h"

parMemberDefinition* parserInfo::FindMember(const char* name)
{
    uint32_t hash = joaat(name);
    if (this == NULL)
        return NULL;
    if (members == NULL)
        return NULL;
    for (parMemberDefinition** member = members; *member != NULL; member++) {
        if ((*member)->name == hash)
            return *member;
    }
    return NULL;
}

uint32_t joaat(const char* key)
{
    uint32_t hash = 0;
    while (*key != '\0') {
        hash += *key;
        key++;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

uint16_t BoneTags[40] = { 24999, 25000, 25001, 25002, 25003, 25004, 25005, 25006, 25007,
    5714, 5715, 5716, 5717, 5718, 5719, 5720, 5721, 5722, 5723, 5698, 5699, 5700, 5701,
    5702, 5703, 5704, 5705, 5706, 5707, 5746, 5747, 5748, 5749, 5750, 5751, 5752, 5753,
    5754, 5755, 5730 };