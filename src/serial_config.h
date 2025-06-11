#pragma once

#ifndef CONFIG_H_20241222
#define CONFIG_H_20241222

#include <Arduino.h>

// Set this to false to disable Serial logging
#define USE_SERIAL true

#if USE_SERIAL == true
#define serial_begin(x)  \
    do                   \
    {                    \
        Serial.begin(x); \
    } while (0)
#define serial_print(x) Serial.print(x)
#define serial_println(x) Serial.println(x)
#else
#define serial_begin(x) \
    do                  \
    {                   \
    } while (0)
#define serial_print(x) ((void)0)
#define serial_println(x) ((void)0)
#endif

#endif // CONFIG_H_20241222