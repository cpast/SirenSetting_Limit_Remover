#include "pch.h"
#include "debug.h"
#include <cstdarg>
#include <cstdio>

#include <strsafe.h>
#include <cassert>

#ifdef SSA_BETA

void logDebug(const char* fmt ...)
{
	va_list args;
	va_start(args, fmt);
	vlog(fmt, args);
}

void flushLog(HANDLE log)
{
	FlushFileBuffers(log);
}

#else

void logDebug(const char* fmt ...)
{
	return;
}

void flushLog(HANDLE log)
{
	return;
}

#endif


HANDLE file = INVALID_HANDLE_VALUE;
bool setup_attempted = false;

bool setup_log()
{
	if (setup_attempted) {
		return (file != INVALID_HANDLE_VALUE);
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
		if (msgBoxMessage == NULL) {
			return false;
		}
		StringCchCat(msgBoxMessage, msgBoxMessageSize, msgBoxPrefix);
		StringCchCat(msgBoxMessage, msgBoxMessageSize, errorMessage);
		MessageBox(NULL, msgBoxMessage, TEXT("Error"), MB_OK);
		LocalFree(msgBoxMessage);
		LocalFree(errorMessage);
		return false;
	}
	return true;
}

void log(const char* fmt ...)
{
	va_list args;
	va_start(args, fmt);
	vlog(fmt, args);
}

void vlog(const char* fmt, va_list args)
{
	if (file == INVALID_HANDLE_VALUE || file == 0)
		if (!setup_log())
			return;
	if (file == INVALID_HANDLE_VALUE || file == 0)
		return;
	char* outputString = NULL;
	SIZE_T outputLen = vsnprintf(outputString, 0, fmt, args);
	outputString = (char*)LocalAlloc(LMEM_ZEROINIT, outputLen + 1);
	vsnprintf(outputString, outputLen + 1, fmt, args);
	WriteFile(file, outputString, outputLen, NULL, NULL);
	LocalFree(outputString);
	flushLog(file);
}

void cleanup_log()
{
	if (file != INVALID_HANDLE_VALUE) {
		FlushFileBuffers(file);
		CloseHandle(file);
	}
	return;
}