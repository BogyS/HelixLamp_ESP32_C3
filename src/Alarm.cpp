#include "Alarm.h"
#include "FSLog.h"
#include "NVSImpl.h"
#include "Config.h"
#include "serial_config.h"

void Alarm::update()
{
    if (!enabled)
        return;
    time_t now;
    time(&now);
    bool doReset = (next <= now);
    bool doAlarm = doReset;
    if (doAlarm && daily)
    {
        long dif = (long)(now - next);
        if (dif > twin)
            doAlarm = false; // to late
    }
    if (doAlarm)
    {
        if (doLog)
        {
            logstr(name.c_str());
            logstr(" activated - ");
            logTime(now);
        }
        else
        {
            serial_print(name.c_str());
            serial_print(" activated - ");
            logBuffTime(now);
            serial_println(LogBuff);
        }
        if (onAlarm != NULL)
            (*onAlarm)(this); // call the function
    }
    if (doReset)
        reset(now);
}

void Alarm::reset(time_t now)
{
    last = now;
    if (daily)
    {
        struct tm tinf;
        localtime_r(&now, &tinf);
        tinf.tm_hour = 23;
        tinf.tm_min = 59;
        tinf.tm_sec = 59;
        next = mktime(&tinf); // 23:59:59
        next++;               // next midnight
        next += inc;
    }
    else
    {
        next = last + inc;
    }
    NVSImpl nvs(nvsn.c_str(), NVS_READWRITE);
    nvs.set_i32("last", (int32_t)last);
    nvs.set_i32("next", (int32_t)next);
    if (doLog)
    {
        logstr("Next time - ");
        logTime(next);
    }
    else
    {
        serial_print("Next time - ");
        logBuffTime(next);
        serial_println(LogBuff);
    }
}

void Alarm::save() const
{
    logstr("Save ");
    NVSImpl nvs(nvsn.c_str(), NVS_READWRITE);
    nvs.set_i32("value", (int32_t)value);
    nvs.set_i32("inc", (int32_t)inc);
    nvs.set_i32("twin", (int32_t)twin);
    nvs.set_u8("daily", daily ? 1 : 0);
    nvs.set_u8("enabled", enabled ? 1 : 0);
    nvs.set_str("name", name);
    nvs.set_i32("last", (int32_t)last);
    nvs.set_i32("next", (int32_t)next);
    logAlarm();
}

void Alarm::load()
{
    logstr("Load ");
    NVSImpl nvs(nvsn.c_str(), NVS_READONLY);
    int32_t i32 = 0;
    if (nvs.get_i32("value", i32))
        value = (int)i32;
    if (nvs.get_i32("inc", i32))
        inc = (long)i32;
    if (nvs.get_i32("twin", i32))
        twin = (long)i32;
    uint8_t ui8 = 0;
    if (nvs.get_u8("daily", ui8))
        daily = (ui8 != 0);
    if (nvs.get_u8("enabled", ui8))
        enabled = (ui8 != 0);
    nvs.get_str("name", name);
    if (nvs.get_i32("last", i32))
        last = (time_t)i32;
    if (nvs.get_i32("next", i32))
        next = (time_t)i32;
    logAlarm();
}

void Alarm::logAlarm() const
{
    logstr(name.c_str());
    logln(" settings:");
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "Value: %d", value);
    logln(LogBuff);
    if (daily)
    {
        logstr("Daily: ");
        logHM(inc);
    }
    else
    {
        LogBuff[0] = 0;
        snprintf(LogBuff, LOG_BUFFER_SIZE, "Fixed interval (s): %li", inc);
        logln(LogBuff);
    }
    logln(enabled ? "Enabled" : "Disabled");
    logstr("Last: ");
    logTime(last);
    logstr("Next: ");
    logTime(next);
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "Time window (s): %li", twin);
    logln(LogBuff);
}

void Alarm::setup(bool clear)
{
    logstr(name.c_str());
    logln(" setup:");
    if (clear)
    {
        time(&last); // from now
        if (daily)
            last -= Alarm::Sec(24, 0, 0); //-25h
        logstr("Clear last: ");
        logTime(last);
    }
    reset(last); // reset the next
}

String Alarm::getNextAsStr() const
{
    struct tm timeinfo;
    localtime_r(&next, &timeinfo);
    LogBuff[0] = 0;
    strftime(LogBuff, LOG_BUFFER_SIZE, "%A, %B %d %Y %H:%M:%S", &timeinfo);
    return String(LogBuff);
}

long Alarm::Sec(int h, int m, int s)
{
    long sec = (long)s;
    sec += ((long)m * 60L);
    sec += ((long)h * 3600L);
    return sec;
}
