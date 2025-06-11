#pragma once

#ifndef WEBCMD_H_20250115
#define WEBCMD_H_20250115

#include <Arduino.h>

// implemented in main
bool broadcastTXT(const char* pStr);
bool broadcastTXT(const char* payload, size_t length);
bool broadcastTXT(String& payload);

#endif // WEBCMD_H_20250115