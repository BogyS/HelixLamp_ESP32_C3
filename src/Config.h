#pragma once

#ifndef CONFIG_H_20230804
#define CONFIG_H_20230804

#include <Arduino.h>

// Place to store all the variables that need to be displayed.
// All other functions should update these!
struct DisplayValues
{
    String ip;
    String rrsi;
    String time;
    String list_state;
    String pos_state;
};
extern DisplayValues DispV;

#endif // CONFIG_H_20230804