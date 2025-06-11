#pragma once

#ifndef _FAux_h_20220719
#define _FAux_h_20220719

#include <Arduino.h>
#include "Alarm.h" //[2023/03/31] - add alarm to restart

// ntp
extern String NTPServer;        //"pool.ntp.org";
extern long GMTOffsetSec;       // 7200
extern int DaylightOffsetSec;   // 3200

extern Alarm RestartAlarm;
extern Alarm SaveTime;   // alarm used to save time once an hour
extern Alarm UpdateTime; // alarm used to update time
extern Alarm EraseLog;

void initAux();
void setupAux();
void saveAux();
void loadAux();
void logAux();
void loopAux();

void saveTimeS();
void loadTimeS();
void updateTime();

#endif //_FAux_h_20220719