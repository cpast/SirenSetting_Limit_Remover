#pragma once

void setup_log();
void logDebug(const char* fmt...);
void log(const char* fmt...);
void cleanup_log();