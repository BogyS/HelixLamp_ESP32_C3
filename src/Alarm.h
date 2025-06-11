#pragma once

#ifndef _Alarm_h_20220620
#define _Alarm_h_20220620

#include <Arduino.h>

// forward decl
class Alarm;
// alarm callback function typedef
typedef void (*OnAlarmFn)(Alarm*);
//[2022/06/20] - alarm class
class Alarm
{
    time_t last;
    time_t next;

public:
    Alarm() : last(0), next(0), onAlarm(NULL)
        , value(18), inc(0), twin(3600)
        , daily(false), enabled(false), doLog(true)
        , name("Alarm"), nvsn("alarm")
    {
    }

    void update();
    void reset(time_t now);
    void save() const;
    void load();
    void setup(bool clear);
    String getNextAsStr() const;
    void logAlarm() const;

    time_t getNext() const { return next; }
    time_t getLast() const { return last; }

    static long Sec(int h, int m, int s);

    OnAlarmFn onAlarm; // if NULL - not enabled
    int value;         // used in callback
    long inc;          // if daily, inc is the time from midnight
    long twin;         // time window for daily alarms def 1h
    bool daily;
    bool enabled;
    bool doLog; // do not write to log
    String name;
    String nvsn; // nvs section name
};

#endif //_Alarm_h_20220620