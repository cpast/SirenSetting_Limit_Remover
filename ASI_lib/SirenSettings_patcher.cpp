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
const pattern GetSirenSetting_Pattern = { "0f b6 c0 48 69 c0 e8 05 00 00 48 03 41", 0x11 };
const pattern GetLightAndSirenIndices_Pattern = { "48 8b 40 48 48 69 c9 e8 05 00 00 8a 0c 01 88 4a 4b", 0x71 };
const pattern RegisterSirenIDs_Pattern = { "48 8b 6e 48 8b fb 48 69 ff e8 05 00 00 80 3c 2f ff", 0x2d };
const pattern CSirenSetting_Clone_Pattern = { "88 01 8b 42 04 48 83 c1 3c 89 41 c8 8b 42 08 48 83 c2 3c 89 41 cc", 0x12 };
const pattern CopyVarToModel_Pattern = { "8a 46 49 49 8b 73 18 88 87 3b 05 00 00 49 8b e3", 0xfb };
const pattern SetFlags_pattern = { "f3 0f 59 c8 f3 0f 2c c1 03 c7 66 0f 6e c0 0f 5b c0 f3 0f 11", 0x35f };
const pattern RphHook_Pattern = { "40 0f b6 c6 69 c0 e8 05 00 00 48 8b 11 48 98", 0xe8 };

bool idHooksAttempted = false;
bool idHooksSucceeded = false;
bool indexHooksAttempted = false;
bool indexHooksSucceeded = false;
bool rphHooksAttempted = false;
bool rphHooksSucceeded = false;
uintptr_t GetSirenSetting_cache = 0;

void ComputeSirenSettings(CVehicleModelInfoVarGlobal* Carcols, CVehicleModelInfoVariation* variations) {
	uint16_t id = 0x100 * variations->field_0x4c;
	id += variations->sirenId;
	log("%s %2.2x %2.2x %4.4x\n", variations->name, variations->field_0x4c, variations->sirenId, id);
	if ((id == 0xff || id == 0xffff) && (variations->sirenIndex < Carcols->sirens.count)) {
		uint16_t realId = (uint16_t)Carcols->sirens.sirens[variations->sirenIndex].Id;
		variations->sirenId = realId & 0xff;
		variations->field_0x4c = realId >> 8;
		log("Index already there: %s %2.2x %2.2x %4.4x\n", variations->name, variations->field_0x4d, variations->sirenIndex, variations->sirenIndex);
		return;
	}
	for (uint16_t i = 0; i < Carcols->sirens.count; i++) {
		if (Carcols->sirens.sirens[i].Id == id) {
			variations->sirenIndex = (uint8_t)i;
			variations->field_0x4d = i >> 8;
			log("Index computed: %s %2.2x %2.2x %4.4x\n", variations->name, variations->field_0x4d, variations->sirenIndex, i);
			return;
		}
	}
	variations->sirenIndex = 0xff;
	variations->field_0x4d = 0;
	log("Index not found: %s %2.2x %2.2x %4.4x\n", variations->name, variations->field_0x4d, variations->sirenIndex, variations->sirenIndex);
}

void LogRegisteredSirens(CVehicleModelInfoVarGlobal* Carcols, uint32_t startingIndex, int length) {
	for (int i = 0; i < length; i++) {
		CSirenSettings* s = Carcols->sirens.sirens + startingIndex + i;
		log("%s %4.4x %4.4x\n", s->Name, s->Id, startingIndex + i);
	}
	log("siren count: %4.4x\n", Carcols->sirens.count);
}

bool ApplyIdHooks(void)
{
	if (idHooksAttempted)
		return idHooksSucceeded;
	idHooksAttempted = true;
	bool success = true;
	uintptr_t SirenSettingParser_Func = FindPattern(CSirenSetting_Parser_Func);
	SirenSettingParser_Func += 9;
	parserInfo* SirenSettingParser = (parserInfo*)(SirenSettingParser_Func - 11 + *(int32_t*)SirenSettingParser_Func);
	log("SirenSettingParser: %p %p\n", SirenSettingParser_Func - 9, SirenSettingParser);
	parserInfo* VariationParser = (parserInfo*)FindPattern(CVehicleModelInfoVariation_Parser);
	log("VariationParser: %p\n", VariationParser);
	uintptr_t GetLightAndSirenIndices = FindPattern(GetLightAndSirenIndices_Pattern);
	log("GetIndex: %p\n", (void*)GetLightAndSirenIndices);
	uintptr_t MergeSirenLists = FindPattern(MergeSirenLists_Pattern);
	log("Merge: %p\n", (void*)MergeSirenLists);
	uintptr_t RegisterSirenIDs = FindPattern(RegisterSirenIDs_Pattern);
	log("Register: %p\n", (void*)RegisterSirenIDs);
	uintptr_t CSirenSetting_Clone = FindPattern(CSirenSetting_Clone_Pattern);
	log("Clone: %p\n", (void*)CSirenSetting_Clone);
	uintptr_t CSirenSetting_Initialize = FindPattern(CSirenSetting_Initialize_Pattern);
	log("Init: %p\n", (void*)CSirenSetting_Initialize);
	success = SirenSettingParser && VariationParser && GetLightAndSirenIndices && MergeSirenLists && RegisterSirenIDs && CSirenSetting_Clone && CSirenSetting_Initialize;
	if (!success)
		return false;
	log("Found patterns\n");

	parMemberDefinition* SirenId = SirenSettingParser->FindMember("id");
	parMemberDefinition* VariationId = VariationParser->FindMember("sirenSettings");
	if (SirenId == NULL || VariationId == NULL)
		return false;
	log("Found members\n");
	SirenId->type = PsoDataType::u32;
	VariationId->type = PsoDataType::u16;
	{
		uint8_t clonea[2] = { 0x8b, 0x02 };
		uint8_t cloneb[2] = { 0x89, 0x01 };
		success = success && WriteForeignMemory(CSirenSetting_Clone + 0xa, clonea, 2);
		success = success && WriteForeignMemory(CSirenSetting_Clone + 0x12, cloneb, 2);
	}
	log("Clone: %s\n", success ? "true" : "false");
	{
		uint8_t reg = 0xc3;
		success = success && InsertHook(RegisterSirenIDs, (uintptr_t) &LogRegisteredSirens); //WriteForeignMemory(RegisterSirenIDs, &reg, 1);
	}
	log("Register: %s\n", success ? "true" : "false");
	{
		SirenSettings_init_ret = (void*) InsertHookWithSkip(CSirenSetting_Initialize + 0x1f, 
			CSirenSetting_Initialize + 0x22, (uintptr_t) &SirenSettings_init_patch);
		success = success && SirenSettings_init_ret;
	}
	log("Init: %s\n", success ? "true" : "false");
	{
		uint8_t mergea[5] = { 0x44, 0x8b, 0x4d, 0x00, 0x90 };
		uint8_t mergeb[3] = { 0x44, 0x39, 0x08 };
		success = success && WriteForeignMemory(MergeSirenLists + 0x42, mergea, 5);
		success = success && WriteForeignMemory(MergeSirenLists + 0x5e, mergeb, 3);
	}
	log("Merge: %s\n", success ? "true" : "false");
	{
		ComputeSirenIndex_logic = &ComputeSirenSettings;
		ComputeSirenIndex_ret = (void*)InsertHookWithSkip(GetLightAndSirenIndices + 0x61,
			GetLightAndSirenIndices + 0x9c, (uintptr_t) &ComputeSirenIndex_patch);
		success = success && ComputeSirenIndex_ret;
	}
	log("Compute: %s\n", success ? "true" : "false");
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
	log("GetSirenSetting: %p\n", GetSirenSetting);
	uintptr_t CopyVarToModel = FindPattern(CopyVarToModel_Pattern);
	log("CopyVarToModel: %p\n", CopyVarToModel);
	uintptr_t SetFlags = FindPattern(SetFlags_pattern);
	log("SetFlags: %p\n", SetFlags);
	success = GetSirenSetting && CopyVarToModel && SetFlags;
	if (!success)
		return false;
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

bool ApplyRphHook(void)
{
	if (rphHooksAttempted)
		return rphHooksSucceeded;
	rphHooksAttempted = true;
	bool success = true;
	success = indexHooksSucceeded;
	if (!success)
		return false;

	uintptr_t RphPrep = 0;
	uintptr_t RphCall = 0;
	uintptr_t RphLogic = 0;

	RphPrep = GetSirenSetting_cache + 0x10;
	RphPrep = *(uintptr_t*)RphPrep;
	log("RphPrep: %p\n", (void*)RphPrep);
	RphCall = RphPrep + 0x2c;
	RphCall += *(int32_t*)(RphPrep + 0x28);
	log("RphCall: %p\n", (void*)RphCall);

	MEMORY_BASIC_INFORMATION mbiRph = { 0 };
	MODULEINFO miRph = { 0 };
	if (VirtualQuery((void*)RphPrep, &mbiRph, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
		return false;
	if (VirtualQuery(mbiRph.AllocationBase, &mbiRph, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
		return false;
	if (GetModuleInformation(GetCurrentProcess(), (HMODULE)mbiRph.BaseAddress, &miRph, sizeof(MODULEINFO)) == false)
		return false;
	log("RphAddress: %p %p %p\n", mbiRph.BaseAddress, mbiRph.RegionSize);
	RphLogic = FindPatternEx((uintptr_t)miRph.lpBaseOfDll, miRph.SizeOfImage, RphHook_Pattern);
	log("RphCall: %p", (void*)RphLogic);
	if (RphLogic == 0)
		return false;

	{
		RphHookPrep_ret = (void*)InsertHook(RphPrep, (uintptr_t)&RphHookPrep_patch);
		uint8_t dxcpy[3] = { 0x0F, 0xB7, 0xD0 };
		success = success && WriteForeignMemory(RphPrep + 0x1a, dxcpy, 3);
	}
	{
		uint8_t r8cpy[4] = { 0x44, 0x0F, 0xB7, 0xC2 };
		success = success && WriteForeignMemory(RphCall + 0x16, r8cpy, 4);
	}
	{
		uint8_t sicpy[4] = { 0x41, 0x0F, 0xB7, 0xF0 };
		success = success && WriteForeignMemory(RphLogic + 0x2a, sicpy, 4);
		uint8_t sicmp[4] = { 0x66, 0x83, 0xFE, 0x01 };
		success = success && WriteForeignMemory(RphLogic + 0x7a, sicmp, 4);
		uint8_t mult[10] = { 0x0F, 0xB7, 0xC6, 0x48, 0x69, 0xC0, 0xE8, 0x05, 0x00, 0x00 };
		success = success && WriteForeignMemory(RphLogic + 0xe8, mult, 10);
		success = success && NopInstruction(RphLogic + 0xf5);
	}

	return success;
}
