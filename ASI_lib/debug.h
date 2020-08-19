#pragma once

bool setup_log();
void logDebug(const char* fmt...);
void log(const char* fmt...);
void cleanup_log();