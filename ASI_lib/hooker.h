#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>

struct pattern {
	const char* pattern;
	const int offset;
};
uintptr_t FindPattern(const char* pattern, const int offset);
uintptr_t FindPattern(const pattern &pattern);
uintptr_t FindPatternEx(uintptr_t start, size_t len, const char* pattern, const int offset);
uintptr_t FindPatternEx(uintptr_t start, size_t len, const pattern &pattern);
uintptr_t NopInstruction(uintptr_t address);
uintptr_t InsertHook(uintptr_t address, uintptr_t hook);
uintptr_t InsertHookWithSkip(uintptr_t branchAddress, uintptr_t returnAddress, uintptr_t hook);
bool WriteForeignMemory(uintptr_t target, void* source, size_t length);