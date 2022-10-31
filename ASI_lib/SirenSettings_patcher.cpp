#include "pch.h"
#include "hooking.h"
#include "SirenSettings_patcher.h"
#include "RageStructs.h"
#include <cassert>
#include <psapi.h>
#include "debug.h"

const pattern CSirenSetting_Parser_Func = { "0f 85 ?? ?? ?? ?? b9 e8 05 00 00 e8", 0xe };
const pattern CVehicleModelInfoVariation_Parser = { "a9 fe ea 18 00 00 00 00 00 00 00 00 00 00 00 00", 0 };
const pattern CSirenSetting_Initialize_Pattern = { "c6 03 ff c7 43 ?? 00 00 80 3f c7 43 ?? 00 00 40 41 c7 43 ?? 00 00 00 41", 0x1F };
const pattern MergeSirenLists_Pattern = { "8b eb 48 69 ed e8 05 00 00 48 03 2e", 0x36 };
const pattern ReallocSirenLists_Pattern = { "0f b7 4b 08 48 8b 6c 24 48 48 8b 74 24 50 8b c1 48 69 c0 e8 05 00 00", 0x9c };
const pattern GetSirenSetting_Pattern = { "0f b6 c0 48 69 c0 e8 05 00 00 48 03 41", 0x11 };
const pattern GetLightAndSirenIndices_Pattern = { "48 8b 40 48 48 69 c9 e8 05 00 00 8a 0c 01 88 4a 4b", 0x71 };
const pattern RegisterSirenIDs_Pattern = { "48 8b 6e 48 8b fb 48 69 ff e8 05 00 00 80 3c 2f ff", 0x2d };
const pattern CSirenSetting_Clone_Pattern = { "88 01 8b 42 04 48 83 c1 3c 89 41 c8 8b 42 08 48 83 c2 3c 89 41 cc", 0x12 };
const pattern CopyVarToModel_Pattern = { "8a 46 49 49 8b 73 18 88 87 3b 05 00 00 49 8b e3", 0xfb };
const pattern SetFlags_pattern = { "f3 0f 59 c8 f3 0f 2c c1 03 c7 66 0f 6e c0 0f 5b c0 f3 0f 11", 0x35f };

bool idHooksAttempted = false;
bool idHooksSucceeded = false;
bool indexHooksAttempted = false;
bool indexHooksSucceeded = false;
uintptr_t GetSirenSetting_cache = 0;

void* (*GtaMalloc)(uint32_t, uint32_t) = NULL;
void (*GtaFree)(void*) = NULL;

void ComputeSirenSettings(CVehicleModelInfoVarGlobal* Carcols, CVehicleModelInfoVariation* variations) {
	uint16_t id = 0x100 * variations->field_0x4c;
	id += variations->sirenId;
	logDebug("%s %2.2x %2.2x %4.4x\n", variations->name, variations->field_0x4c, variations->sirenId, id);
	if ((id == 0xff || id == 0xffff) && (variations->sirenIndex < Carcols->sirens.count)) {
		uint16_t realId = (uint16_t)Carcols->sirens.sirens[variations->sirenIndex].Id;
		variations->sirenId = realId & 0xff;
		variations->field_0x4c = realId >> 8;
		variations->field_0x4d = 0;
		logDebug("Index already there: %s %2.2x %2.2x %4.4x\n", variations->name, variations->field_0x4d, variations->sirenIndex, variations->sirenIndex);
		log("Vehicle %s: id %i and index %i means new id %i\n", variations->name, id, variations->sirenIndex, realId);
		return;
	}
	for (uint16_t i = 0; i < Carcols->sirens.count; i++) {
		if ((uint16_t)(Carcols->sirens.sirens[i].Id) == id) {
			variations->sirenIndex = (uint8_t)i;
			variations->field_0x4d = i >> 8;
			logDebug("Index computed: %s %2.2x %2.2x %4.4x\n", variations->name, variations->field_0x4d, variations->sirenIndex, i);
			log("Vehicle %s: %i -> %i\n", variations->name, id, i);
			return;
		}
	}
	variations->sirenIndex = 0x0;
	variations->field_0x4d = 0;
	logDebug("Index not found: %s %2.2x %2.2x %4.4x\n", variations->name, variations->field_0x4d, variations->sirenIndex, variations->sirenIndex);
	log("Vehicle %s: %i NOT FOUND -> %i\n", variations->name, id, variations->sirenIndex);
}

void LogRegisteredSirens(CVehicleModelInfoVarGlobal* Carcols, uint32_t startingIndex, int length) {
	for (int i = 0; i < length; i++) {
		CSirenSettings* s = Carcols->sirens.sirens + startingIndex + i;
		const char* siren_name = s->Name;
		if (siren_name == NULL)
			siren_name = "UNNAMED";
		logDebug("%s %4.4x %4.4x\n", siren_name, s->Id, startingIndex + i);
		log("SirenSetting %s: %i at %i\n", siren_name, (uint16_t)(s->Id), startingIndex + i);
	}
	logDebug("siren count: %4.4x\n", Carcols->sirens.count);
}

void LogConflict(CSirenSettings* siren1, CSirenSettings* siren2) {
	const char* siren_1_name = siren1->Name;
	if (siren_1_name == NULL)
		siren_1_name = "UNNAMED";
	const char* siren_2_name = siren2->Name;
	if (siren_2_name == NULL)
		siren_2_name = "UNNAMED";
	log("CONFLICT: %s and %s share ID %i\n", siren_1_name, siren_2_name, (uint16_t)(siren1->Id));
}

bool ApplyIdHooks(void)
{
	if (idHooksAttempted)
		return idHooksSucceeded;
	idHooksAttempted = true;
	bool success = true;
	uintptr_t SirenSettingParser_Func = FindPattern(CSirenSetting_Parser_Func);
	parserInfo* SirenSettingParser = NULL;
	if (SirenSettingParser_Func != NULL) {
		int32_t* parserOffset = (int32_t*)(SirenSettingParser_Func + 0x9);
		uintptr_t parserRefBase = SirenSettingParser_Func + 0xe;
		SirenSettingParser = (parserInfo*)(parserRefBase + *parserOffset - 0x10);
	}
	logDebug("SirenSettingParser: %p %p\n", SirenSettingParser_Func, SirenSettingParser);
	parserInfo* VariationParser = (parserInfo*)FindPattern(CVehicleModelInfoVariation_Parser);
	logDebug("VariationParser: %p\n", VariationParser);
	uintptr_t GetLightAndSirenIndices = FindPattern(GetLightAndSirenIndices_Pattern);
	logDebug("GetIndex: %p\n", (void*)GetLightAndSirenIndices);
	uintptr_t MergeSirenLists = FindPattern(MergeSirenLists_Pattern);
	logDebug("Merge: %p\n", (void*)MergeSirenLists);
	uintptr_t ReallocSirenLists = FindPattern(ReallocSirenLists_Pattern);
	if (ReallocSirenLists != NULL)
	{
		GtaFree = (void(*)(void*))GetReferencedAddress(ReallocSirenLists + 0x94);
		GtaMalloc = (void*(*)(uint32_t, uint32_t))GetReferencedAddress(ReallocSirenLists + 0x43);
	}
	logDebug("Realloc: %p\n", (void*)ReallocSirenLists);
	uintptr_t RegisterSirenIDs = FindPattern(RegisterSirenIDs_Pattern);
	logDebug("Register: %p\n", (void*)RegisterSirenIDs);
	uintptr_t CSirenSetting_Clone = FindPattern(CSirenSetting_Clone_Pattern);
	logDebug("Clone: %p\n", (void*)CSirenSetting_Clone);
	uintptr_t CSirenSetting_Initialize = FindPattern(CSirenSetting_Initialize_Pattern);
	logDebug("Init: %p\n", (void*)CSirenSetting_Initialize);
	success = SirenSettingParser && VariationParser && GetLightAndSirenIndices && MergeSirenLists && RegisterSirenIDs && CSirenSetting_Clone && CSirenSetting_Initialize
		&& ReallocSirenLists && GtaFree && GtaMalloc;
	if (!success)
		return false;
	log("Found ID patterns.\n");

	parMemberDefinition* SirenId = SirenSettingParser->FindMember("id");
	parMemberDefinition* VariationId = VariationParser->FindMember("sirenSettings");
	parMemberArrayDefinition* SirenLights = (parMemberArrayDefinition*) SirenSettingParser->FindMember("sirens");

	if (SirenId == NULL || VariationId == NULL || SirenLights == NULL)
		return false;
	log("Found ID members.\n");
	SirenId->type = PsoDataType::u32;
	VariationId->type = PsoDataType::u16;
	SirenLights->subtype = parMemberArraySubtype::ATARRAY;
	SirenLights->offset = 0x40;
	for (int i = 0; i < 20; i++)
		if (SirenSettingParser->offsets[i] == 0x3c)
			SirenSettingParser->offsets[i] = 0x40;
	{
		success = success && InsertHook(CSirenSetting_Clone, (uintptr_t)CloneSirenSettings);
	}
	logDebug("Clone: %s\n", success ? "true" : "false");
	{
		uint8_t reg = 0xc3;
		success = success && InsertHook(RegisterSirenIDs, (uintptr_t) &LogRegisteredSirens); //WriteForeignMemory(RegisterSirenIDs, &reg, 1);
	}
	logDebug("Register: %s\n", success ? "true" : "false");
	{
		SirenSettings_init_ret = (void*) InsertHookWithSkip(CSirenSetting_Initialize + 0x1f, 
			CSirenSetting_Initialize + 0x22, (uintptr_t) &SirenSettings_init_patch);
		success = success && SirenSettings_init_ret;
		success = success && NopInstruction(CSirenSetting_Initialize + 0x11);
		SirenSettings_Free_ret = (void*) InsertNearHookWithSkip(SirenSettingParser_Func + 0x1e, SirenSettingParser_Func + 0x25, (uintptr_t)&SirenSettings_Free_patch);
		success = success && SirenSettings_Free_ret;
	}
	logDebug("Init: %s\n", success ? "true" : "false");
	{
		uint8_t mergea[5] = { 0x44, 0x8b, 0x4d, 0x00, 0x90 };
		uint8_t mergeb[3] = { 0x44, 0x39, 0x08 };
		success = success && WriteForeignMemory(MergeSirenLists + 0x42, mergea, 5);
		success = success && WriteForeignMemory(MergeSirenLists + 0x5e, mergeb, 3);
		uint8_t rel_jmp = *(uint8_t*)(MergeSirenLists + 0x62);
		LogConflict_z_ret = (void*)(MergeSirenLists + 0x63 + rel_jmp);
		LogConflict_nz_ret = (void*)InsertHookWithSkip(MergeSirenLists + 0x61,
			MergeSirenLists + 0x63, (uintptr_t)&LogConflict_patch);
		success = success && LogConflict_nz_ret;
		SirenSettings_ReallocFree_ret = (void*)InsertNearHookWithSkip(ReallocSirenLists + 0x91, ReallocSirenLists + 0x99, (uintptr_t)&SirenSettings_ReallocFree_patch);
		success = success && SirenSettings_ReallocFree_ret;
	}
	logDebug("Merge: %s\n", success ? "true" : "false");
	{
		ComputeSirenIndex_logic = &ComputeSirenSettings;
		ComputeSirenIndex_ret = (void*)InsertHookWithSkip(GetLightAndSirenIndices + 0x61,
			GetLightAndSirenIndices + 0x9c, (uintptr_t) &ComputeSirenIndex_patch);
		success = success && ComputeSirenIndex_ret;
	}
	logDebug("Compute: %s\n", success ? "true" : "false");
	idHooksSucceeded = success;
	return success;
}

bool ApplyIndexHooks(void)
{
	if (indexHooksAttempted)
		return indexHooksSucceeded;
	indexHooksAttempted = true;
	bool success = true;
	success = ApplyIdHooks();
	if (!success)
		return false;

	uintptr_t GetSirenSetting = FindPattern(GetSirenSetting_Pattern);
	GetSirenSetting_cache = GetSirenSetting;
	logDebug("GetSirenSetting: %p\n", GetSirenSetting);
	uintptr_t CopyVarToModel = FindPattern(CopyVarToModel_Pattern);
	logDebug("CopyVarToModel: %p\n", CopyVarToModel);
	uintptr_t SetFlags = FindPattern(SetFlags_pattern);
	logDebug("SetFlags: %p\n", SetFlags);
	success = GetSirenSetting && CopyVarToModel && SetFlags;
	if (!success)
		return false;
	log("Found index patterns.");

	int32_t* carcols_mov = (int32_t*)(GetSirenSetting + 0xd);
	uintptr_t carcols = GetSirenSetting + 0x11;
	carcols += *carcols_mov;
	CarCols_loc = (void*)carcols;
	{
		GetSirenSetting_ret = (void*)InsertHookWithSkip(GetSirenSetting + 0x6,
			GetSirenSetting + 0x22, (uintptr_t) &GetSirenSetting_patch);
		success = success && GetSirenSetting_ret;		
	}
	{
		CopyVarToModel_ret = (void*)InsertHook(CopyVarToModel + 0xfb, (uintptr_t) &CopyVarToModel_patch);
		success = success && CopyVarToModel_ret;
	}
	{
		SetFlags_ret = (void*)InsertHookWithSkip(SetFlags + 0x577,
			SetFlags + 0x57e, (uintptr_t)&SetFlags_patch);
		success = success && SetFlags_ret;
	}
	indexHooksSucceeded = success;
	return success;
}

int EnableRphIntegration(void* rphFunc)
{
	if (rphFunc == NULL)
		return 0;
	RphOnGetSirenSettings = rphFunc;
	uintptr_t RPHPatchLoc = (uintptr_t)&GetSirenSetting_RPH_patch;
	WriteForeignMemory(GetSirenSetting_cache + 0x6 + 0x6, &RPHPatchLoc, 8);
	return NUM_LIGHTS_SUPPORTED;
}

CSirenSettings* CloneSirenSettings(CSirenSettings* dst, CSirenSettings* src)
{
	memcpy(dst, src, sizeof(CSirenSettings));
	dst->Lights = (EmergencyLight*)GtaMalloc(sizeof(EmergencyLight) * NUM_LIGHTS_SUPPORTED, 0x10);
	dst->lightSize = NUM_LIGHTS_SUPPORTED;
	dst->lightCount = NUM_LIGHTS_SUPPORTED;
	dst->NumSirens = NUM_LIGHTS_SUPPORTED;
	int copyCount = NUM_LIGHTS_SUPPORTED;
	if (src->lightSize < copyCount)
		copyCount = src->lightSize;
	memcpy(dst->Lights, src->Lights, sizeof(EmergencyLight) * copyCount);
	if (copyCount < NUM_LIGHTS_SUPPORTED)
		memset(dst->Lights + copyCount, NULL, sizeof(EmergencyLight) * (NUM_LIGHTS_SUPPORTED - copyCount));
	return dst;
}

void FreeSirenSettingsAtArray(CSirenSettings_atArray* arr)
{
	for (int i = 0; i < arr->count; i++)
		GtaFree(arr->sirens[i].Lights);
	GtaFree(arr->sirens);
}

void FreeSirenSettings(CSirenSettings* toFree)
{
	if (toFree->lightSize != 0)
		GtaFree(toFree->Lights);
}