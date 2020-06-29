#include "pch.h"
#include "debug.h"
#include <cstdarg>
#include <cstdio>

#ifdef SSA_BETA

FILE* file = NULL;

void setup_log()
{
	fopen_s(&file, "siren.log", "w");
}

void logDebug(const char* fmt ...)
{
	va_list args;
	va_start(args, fmt);
	if (file == NULL)
		setup_log();
	vfprintf(file, fmt, args);
	fflush(file);
}

void log(const char* fmt ...)
{
	va_list args;
	va_start(args, fmt);
	logDebug(fmt, args);
}

void cleanup_log()
{
	fclose(file);
}

#else
#include <strsafe.h>

HANDLE file = INVALID_HANDLE_VALUE;
bool setup_attempted = false;

void setup_log()
{
	if (setup_attempted) {
		return;
	}
	file = CreateFile(TEXT("SirenSettings.log"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		LPTSTR errorMessage = NULL;
		LPTSTR msgBoxMessage = NULL;
		TCHAR msgBoxPrefix[] = TEXT("SirenSetting_Limit_Adjuster log creation failed with error:\n");
		DWORD errorCode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0, (LPTSTR)&errorMessage, 0, NULL);
		SIZE_T msgBoxMessageSize = sizeof(TCHAR) * ((SIZE_T)lstrlen(errorMessage) + (SIZE_T)lstrlen(msgBoxPrefix) + 1);
		msgBoxMessage = (LPTSTR) LocalAlloc(LMEM_ZEROINIT, msgBoxMessageSize);
		StringCchCat(msgBoxMessage, msgBoxMessageSize, msgBoxPrefix);
		StringCchCat(msgBoxMessage, msgBoxMessageSize, errorMessage);
		MessageBox(NULL, msgBoxMessage, TEXT("Error"), MB_OK);
		LocalFree(msgBoxMessage);
		LocalFree(errorMessage);
	}
}

void log(const char* fmt ...)
{
	va_list args;
	va_start(args, fmt);
	if (file == INVALID_HANDLE_VALUE)
		setup_log();
	char* outputString = NULL;
	SIZE_T outputLen = vsnprintf(outputString, 0, fmt, args);
	outputString = (char*)LocalAlloc(LMEM_ZEROINIT, outputLen + 1);
	vsnprintf(outputString, outputLen+1, fmt, args);
	WriteFile(file, outputString, outputLen, NULL, NULL);
	LocalFree(outputString);
}

void logDebug(const char* fmt ...)
{
	return;
}

void cleanup_log()
{
	if (file != INVALID_HANDLE_VALUE) {
		FlushFileBuffers(file);
		CloseHandle(file);
	}
	return;
}

#endif