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

uint16_t SirenBoneTags[64] = { 24999, 25000, 25001, 25002, 25003, 25004, 25005, 25006, \
                               25007,  5714,  5715,  5716,  5717,  5718,  5719,  5720, \
                                5721,  5722,  5723,  5698,  5699,  5700,  5701,  5702, \
                                5703,  5704,  5705,  5706,  5707,  5746,  5747,  5748, \
                                5749,  5750,  5751,  5752,  5753,  5754,  5755,  5730, \
                                5731,  5732,  5733,  5734,  5735,  5736,  5737,  5738, \
                                5739,  5906,  5907,  5908,  5909,  5910,  5911,  5912, \
                                5913,  5914,  5915,  5890,  5891,  5892,  5893,  5894 };


uint16_t GlassBoneTags[64] = { 63112, 63113, 63114, 63115, 63116, 63117, 63118, 63119, \
                               63120, 16452, 16453, 16454, 16455, 16456, 16457, 16458, \
                               16459, 16460, 16461, 16468, 16469, 16470, 16471, 16472, \
                               16473, 16474, 16475, 16476, 16477, 16420, 16421, 16422, \
                               16423, 16424, 16425, 16426, 16427, 16428, 16429, 16436, \
                               16437, 16438, 16439, 16440, 16441, 16442, 16443, 16444, \
                               16445, 16644, 16645, 16646, 16647, 16648, 16649, 16650, \
                               16651, 16652, 16653, 16660, 16661, 16662, 16663, 16664 };