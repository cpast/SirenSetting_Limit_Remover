#include "pch.h"
#include "RageStructs.h"
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
