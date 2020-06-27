#pragma once
//#ifdef _DEBUG
#define LOG
//#endif

void setup_log();
void log(const char* fmt...);
void cleanup_log();