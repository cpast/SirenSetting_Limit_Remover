#pragma once
//#define SSA_BETA
bool setup_log();
void logDebug(const char* fmt...);
void log(const char* fmt...);
void vlog(const char* fmt, va_list args);
void cleanup_log();