#pragma once

#ifndef _FSLog_h_20230412
#define _FSLog_h_20230412

#include "FSDef.h"

const char LogFile[] = "/log.txt";
#define LOG_BUFFER_SIZE 512
extern char LogBuff[LOG_BUFFER_SIZE];

extern bool localTimeOK;

void resetLog();
void setupLog(fs::FS& fs);
void logstr(const char *msg); // print/ln
void logln();
void logln(const char *msg);
void logLocalTime();        // update localTimeOK
void logValidTime();        // update localTimeOK and log only valid, doesnt wait
void logBuffTime(time_t t); // write time in LogBuff
void logTime(time_t t);
void logBuffHM(time_t t);
void logHM(time_t t);
void logHM();
void logBuffHMS(time_t t);
void logHMS(time_t t);
void logHMS();

void logFS();
void logMem();
//verify if file exist
bool checkFile( const char *path, size_t minSize = 100 );

const char* formatBytes(size_t bytes);

#endif  //_FSLog_h_20230412