#include "pch.h"
#include "debug.h"
#ifdef LOG
#include <cstdio>
#include <cstdarg>

FILE* file = NULL;

void setup_log()
{
	fopen_s(&file, "siren.log", "w");
}

void log(const char* fmt ...)
{
	va_list args;
	va_start(args, fmt);
	if (file == NULL)
		setup_log();
	vfprintf(file, fmt, args);
	fflush(file);
}

void cleanup_log()
{
	fclose(file);
}

#else

void setup_log()
{
	return;
}

void log(const char* fmt ...)
{
	return;
}

void cleanup_log()
{
	return;
}

#endif