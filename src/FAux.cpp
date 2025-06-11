#include "FAux.h"
#include "FSLog.h"
#include "NVSImpl.h"

// ntp
String NTPServer("pool.ntp.org");
long GMTOffsetSec = 7200;
int DaylightOffsetSec = 3600;

Alarm RestartAlarm;
Alarm SaveTime;
Alarm UpdateTime;
Alarm EraseLog;

void restartImpl(Alarm* pAlarm)
{
    logln("Restart!");
    logLocalTime();
    ESP.restart();
}

void updateTimeImpl(Alarm* pAlarm)
{
    logln("Update time");
    configTime(GMTOffsetSec, DaylightOffsetSec, NTPServer.c_str());
    logstr("Local time: ");
    logLocalTime(); // wait for time
}

void eraseLogImpl(Alarm* pAlarm)
{
    resetLog();
}

void initAux()
{
    // init the alarms
    RestartAlarm.onAlarm = restartImpl;
    RestartAlarm.inc = Alarm::Sec(0, 15, 0);
    RestartAlarm.daily = false;
    RestartAlarm.enabled = false;
    RestartAlarm.name = "On CP restart";
    RestartAlarm.nvsn = "alarm_cpr";

    SaveTime.inc = Alarm::Sec(1, 0, 0);
    SaveTime.daily = false;
    SaveTime.enabled = true;
    SaveTime.name = "Save time";
    SaveTime.nvsn = "alarm_t";
    SaveTime.doLog = false; // do not write to log onAlarm

    UpdateTime.onAlarm = updateTimeImpl;
    UpdateTime.inc = Alarm::Sec(6, 0, 0);
    UpdateTime.daily = false;
    UpdateTime.enabled = true;
    UpdateTime.name = "Update time";
    UpdateTime.nvsn = "alarm_tu";
    UpdateTime.doLog = false; // do not write to log onAlarm

    //EraseLog
    EraseLog.onAlarm = eraseLogImpl;
    EraseLog.inc = Alarm::Sec(24 * 7, 0, 0);
    EraseLog.daily = true;
    EraseLog.enabled = true;
    EraseLog.name = "Reset log";
    EraseLog.nvsn = "a_resetlog";
}

void setupAux()
{
}

void saveAux()
{
    logln("Save aux settings:");
    NVSImpl nvs("f_aux", NVS_READWRITE);

    RestartAlarm.save();
    SaveTime.save();
    UpdateTime.save();
    EraseLog.save();

    logAux();
}

void saveTimeS()
{
    logln("Save time settings:");
    NVSImpl nvs("f_aux", NVS_READWRITE);

    SaveTime.save();
    UpdateTime.save();
}

void loadAux()
{
    logln("Load aux settings:");
    NVSImpl nvs("f_aux", NVS_READONLY);

    RestartAlarm.load();
    RestartAlarm.enabled = false; // just to be sure

    SaveTime.load();
    UpdateTime.load();
    EraseLog.load();

    logAux();
}

void loopAux()
{
    RestartAlarm.update();
    SaveTime.update();
    UpdateTime.update();
    EraseLog.update();
}

void loadTimeS()
{
    NVSImpl nvs("f_aux", NVS_READONLY);
    SaveTime.load();
    UpdateTime.load();
}

void logAux()
{
}

void FFSetTimeZone(long offset, int daylight)
{
    char cst[17] = {0};
    char cdt[17] = "DST";
    char tz[33] = {0};

    if (offset % 3600)
    {
        snprintf(cst, 17, "UTC%ld:%02u:%02u", offset / 3600, (unsigned int)abs((offset % 3600) / 60), (unsigned int)abs(offset % 60));
    }
    else
    {
        snprintf(cst, 17, "UTC%ld", offset / 3600);
    }
    if (daylight != 3600)
    {
        long tz_dst = offset - daylight;
        if (tz_dst % 3600)
        {
            snprintf(cdt, 17, "DST%ld:%02u:%02u", tz_dst / 3600, (unsigned int)abs((tz_dst % 3600) / 60), (unsigned int)abs(tz_dst % 60));
        }
        else
        {
            snprintf(cdt, 17, "DST%ld", tz_dst / 3600);
        }
    }
    snprintf(tz, 33, "%s%s", cst, cdt);
    setenv("TZ", tz, 1);
    tzset();

    logstr("Time zone: ");
    logln(tz);
}
///////////////////////////////////////////////////////////////////////////
bool VerifyYear(time_t t)
{
    struct tm timeinfo;
    localtime_r(&t, &timeinfo);
    return (timeinfo.tm_year > (2021 - 1900));
}

void updateTime()
{
    time_t now;
    time(&now);
    if (VerifyYear(now))
        return; // time ok
    // load last saved
    now = SaveTime.getLast();
    if (!VerifyYear(now))
    {
        // ups - no time was set
        logln("No valid time record!");
        SaveTime.enabled = false; // no need to save invalid time
    }
    else
    {
        // now has last saved time
        struct timeval tv = {.tv_sec = now, .tv_usec = 0};
        settimeofday(&tv, NULL);
        FFSetTimeZone(-GMTOffsetSec, DaylightOffsetSec);
        logln("Time updated from max time record.");
    }
    logstr("Time updated to : ");
    time(&now); //[2023/07/31] - re-read the time
    logTime(now);
}
