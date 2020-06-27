#include "pch.h"
#include "hooker.h"
#define NMD_LDISASM_IMPLEMENTATION
#include "nmd_ldisasm.h"
#include <Windows.h>
#include <cstdlib>
#include "debug.h"

uintptr_t exeStart = NULL;
size_t exeLen = 0;

HANDLE heap = NULL;

bool EnsureExe()
{
	if (exeStart == NULL) {
		MODULEINFO modInfo = { 0 };
		HANDLE currProc = GetCurrentProcess();
		HMODULE mainExe = GetModuleHandle(NULL);
		if (GetModuleInformation(currProc, mainExe, &modInfo, sizeof(MODULEINFO)) == 0)
			return false;
		exeStart = (uintptr_t)modInfo.lpBaseOfDll;
		exeLen = (size_t)modInfo.SizeOfImage;
		log("Module: %p %p\n", exeStart, exeLen);
	}
	return true;
}

bool EnsureHeap()
{
	if (heap == NULL) {
		heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0x1000, 0x5000);
		if (heap == NULL)
			return false;
	}
	return true;
}

bool memptn(const uint8_t* mem, const uint8_t* ptn, const uint8_t* mask, size_t len)
{
	for (int i = 0; i < len; i++) {
		if ((mask[i] & mem[i]) != (mask[i] & ptn[i]))
			return false;
	}
	return true;
}

void CompilePattern(const char* ptnStr, uint8_t** ptnBytes, uint8_t** maskBytes, size_t* ptnLen)
{
	int nibbleCount = 0;
	size_t ptnStrLen = strlen(ptnStr);
	for (size_t i = 0; i < ptnStrLen; i++) {
		if ('0' <= ptnStr[i] && ptnStr[i] <= '9')
			nibbleCount++;
		else if ('a' <= ptnStr[i] && ptnStr[i] <= 'f')
			nibbleCount++;
		else if ('A' <= ptnStr[i] && ptnStr[i] <= 'F')
			nibbleCount++;
		else if (ptnStr[i] == '.' || ptnStr[i] == '*' || ptnStr[i] == '?')
			nibbleCount++;
	}
	int byteCount = (nibbleCount + 1) / 2;
	uint8_t* pattern = (uint8_t*)malloc(byteCount);
	uint8_t* mask = (uint8_t*)malloc(byteCount);
	memset(mask, 0, byteCount);
	memset(pattern, 0, byteCount);
	int ptnIdx = 0;
	int offset = 4;
	for (int i = 0; i < ptnStrLen; i++) {
		if ('0' <= ptnStr[i] && ptnStr[i] <= '9') {
			mask[ptnIdx / 2] |= 0xf << offset;
			pattern[ptnIdx / 2] |= (ptnStr[i] - '0') << offset;
			ptnIdx++;
			offset ^= 4;
		}
		else if ('a' <= ptnStr[i] && ptnStr[i] <= 'f') {
			mask[ptnIdx / 2] |= 0xf << offset;
			pattern[ptnIdx / 2] |= (10 + ptnStr[i] - 'a') << offset;
			ptnIdx++;
			offset ^= 4;
		}
		else if ('A' <= ptnStr[i] && ptnStr[i] <= 'F') {
			mask[ptnIdx / 2] |= 0xf << offset;
			pattern[ptnIdx / 2] |= (10 + ptnStr[i] - 'A') << offset;
			ptnIdx++;
			offset ^= 4;
		}
		else if (ptnStr[i] == '.' || ptnStr[i] == '*' || ptnStr[i] == '?') {
			ptnIdx++;
			offset ^= 4;
		}
	}
	*ptnBytes = pattern;
	*maskBytes = mask;
	*ptnLen = byteCount;
}

uintptr_t FindPattern(const char* pattern, const int offset)
{
	if (!EnsureExe())
		return NULL;
	return FindPatternEx(exeStart, exeLen, pattern, offset);
}

uintptr_t FindPattern(const pattern& pattern)
{
	return FindPattern(pattern.pattern, pattern.offset);
}

uintptr_t FindPatternEx(uintptr_t start, size_t len, const char* ptnStr, const int offset)
{
	size_t skip[256];
	uint8_t* pattern;
	uint8_t* mask;
	uint8_t* buffer = (uint8_t*)start;
	size_t patternLength;
	CompilePattern(ptnStr, &pattern, &mask, &patternLength);
	/*log("Pattern: %s\nCompiled: ", ptnStr);
	for (int i = 0; i < patternLength; i++)
		log("%2.2x ", pattern[i]);
	log("\nMask: ");
	for (int i = 0; i < patternLength; i++)
		log("%2.2x ", mask[i]);
	log("\n");*/
	for (int i = 0; i < 256; i++) {
		skip[i] = patternLength;
	}
	for (int i = 0; i < patternLength - 1; i++) {
		size_t skipVal = patternLength - 1 - i;
		if (mask[i] == 0xff)
			skip[pattern[i]] = skipVal;
		else
			for (int j = 0; j < 256; j++)
				if ((mask[i] & j) == (mask[i] & pattern[i]))
					skip[j] = skipVal;
	}
	for (size_t idx = 0; idx + patternLength <= len; idx += skip[buffer[idx + patternLength - 1]]) {
		if (memptn(buffer + idx, pattern, mask, patternLength)) {
			free(pattern);
			free(mask);
			return start + idx - offset;
		}
	}
	free(pattern);
	free(mask);
	return NULL;
}

uintptr_t FindPatternEx(uintptr_t start, size_t len, const pattern& pattern)
{
	return FindPatternEx(start, len, pattern.pattern, pattern.offset);
}

uintptr_t NopInstruction(uintptr_t address)
{
	size_t length = nmd_x86_ldisasm((void*)address, NMD_LDISASM_X86_MODE_64);
	void* nops = malloc(length);
	memset(nops, 0x90, length);
	bool success = WriteForeignMemory(address, nops, length);
	free(nops);
	if (!success)
		return NULL;
	return address + length;
}

uintptr_t InsertHook(uintptr_t address, uintptr_t hook)
{
	return InsertHookWithSkip(address, address, hook);
}

bool WriteLongJump(uintptr_t from, uintptr_t target) {
	uint8_t jmp[6 + sizeof(uint64_t)] = { 0 };
	jmp[0] = 0xff;
	jmp[1] = 0x25;
	uint64_t targetAddr = (uint64_t)target;
	memcpy(jmp + 6, &targetAddr, sizeof(uint64_t));
	return WriteForeignMemory(from, jmp, 6 + sizeof(uint64_t));
}

uintptr_t InsertHookWithSkip(uintptr_t branchAddress, uintptr_t returnAddress, uintptr_t hook)
{
	uintptr_t actualRetAddr = returnAddress;
	size_t skipLength = returnAddress - branchAddress;
	size_t minHookLength = 0xe;
	size_t clobberLength = 0;
	while (clobberLength < minHookLength) {
		size_t nextInstrLength = nmd_x86_ldisasm((void*)(branchAddress + clobberLength), NMD_LDISASM_X86_MODE_64);
		if (nextInstrLength == 0)
			return NULL;
		clobberLength += nextInstrLength;
	}
	size_t copyLength = 0;
	if (clobberLength > skipLength) {
		copyLength = clobberLength - skipLength;
	}

	if (copyLength > 0) {
		if (!EnsureHeap())
			return NULL;
		actualRetAddr = (uintptr_t)HeapAlloc(heap, 0, copyLength + minHookLength);
		if (actualRetAddr == NULL)
			return NULL;
		memcpy((void*)actualRetAddr, (void*)returnAddress, copyLength);
		if (!WriteLongJump(actualRetAddr + copyLength, returnAddress + copyLength)) {
			HeapFree(heap, 0, (void*)actualRetAddr);
			return NULL;
		}
	}

	if (!WriteLongJump(branchAddress, hook))
		return NULL;
	return actualRetAddr;
}

bool WriteForeignMemory(uintptr_t target, void* source, size_t length)
{
	DWORD oldProtect = 0;
	DWORD newProtect = 0;
	if (!VirtualProtect((void*)target, length, PAGE_EXECUTE_READWRITE, &oldProtect))
		return false;
	memcpy((void*)target, source, length);
	if (!VirtualProtect((void*)target, length, oldProtect, &newProtect))
		return false;
	return true;
}
