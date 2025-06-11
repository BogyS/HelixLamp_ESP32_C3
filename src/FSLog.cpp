#include "FSLog.h"
#include "Config.h"
#include "serial_config.h"

bool HasSPIFFS = false;
char LogBuff[LOG_BUFFER_SIZE];
bool localTimeOK = false;

fs::FS* _logFS = nullptr;

void setupLog(fs::FS& fs)
{
    LogBuff[0] = 0;
    _logFS = &fs;
    if (!_logFS)
        return;
    if (_logFS->exists(LogFile))
        return;
    resetLog();
}

void resetLog()
{
    if (!_logFS)
        return;
    File f = _logFS->open(LogFile, "w");
    if (!f)
    {
        serial_println("Log file open failed");
        return;
    }
    f.println("//Log file");
    f.close();
}

void logstr(const char *msg)
{
    serial_print(msg);
    if (!_logFS)
        return;
    File f = _logFS->open(LogFile, "a");
    if (!f)
    {
        serial_println("Log file open failed");
        return;
    }
    f.print(msg);
    f.close();
}
void logln(const char *msg)
{
    serial_println(msg);
    if (!_logFS)
        return;
    File f = _logFS->open(LogFile, "a");
    if (!f)
    {
        serial_println("Log file open failed");
        return;
    }
    f.println(msg);
    f.close();
}

void logln()
{
    logln("");
}

void logLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        logln("Failed to obtain time");
        localTimeOK = false;
    }
    LogBuff[0] = 0;
    strftime(LogBuff, LOG_BUFFER_SIZE, "%A, %B %d %Y %H:%M:%S", &timeinfo);
    logln(LogBuff);
    localTimeOK = true;
}

// update localTimeOK and log only valid, doesnt wait
void logValidTime()
{
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    if(timeinfo.tm_year < (2016 - 1900))
        return;
    logTime(now);
    localTimeOK = true;
}

void logBuffTime(time_t t)
{ // write time in LogBuff
    struct tm timeinfo;
    localtime_r(&t, &timeinfo);
    LogBuff[0] = 0;
    strftime(LogBuff, LOG_BUFFER_SIZE, "%A, %B %d %Y %H:%M:%S", &timeinfo);
}

void logTime(time_t t)
{
    logBuffTime(t);
    logln(LogBuff);
}

void logBuffHM(time_t t)
{
    struct tm timeinfo;
    localtime_r(&t, &timeinfo);
    LogBuff[0] = 0;
    strftime(LogBuff, LOG_BUFFER_SIZE, "%H:%M", &timeinfo);
}

void logHM(time_t t)
{
    logBuffHM(t);
    logln(LogBuff);
}

void logHM()
{
    time_t t;
    time(&t);
    logBuffHM(t);
    logln(LogBuff);
}

void logBuffHMS(time_t t)
{
    struct tm timeinfo;
    localtime_r(&t, &timeinfo);
    LogBuff[0] = 0;
    strftime(LogBuff, LOG_BUFFER_SIZE, "%H:%M:%S", &timeinfo);
}

void logHMS(time_t t)
{
    logBuffHM(t);
    logln(LogBuff);
}

void logHMS()
{
    time_t t;
    time(&t);
    logBuffHMS(t);
    logln(LogBuff);
}

// format bytes
const char* formatBytes(size_t bytes)
{
    LogBuff[0] = 0;
    if (bytes < 1024)
        snprintf(LogBuff, LOG_BUFFER_SIZE, "%uB", (uint32_t)bytes);
    else if (bytes < (1024 * 1024))
        snprintf(LogBuff, LOG_BUFFER_SIZE, "%uKB", (uint32_t)(bytes / 1024.0));
    else if (bytes < (1024 * 1024 * 1024))
        snprintf(LogBuff, LOG_BUFFER_SIZE, "%uMB", (uint32_t)(bytes / 1024.0 / 1024.0));
    else
        snprintf(LogBuff, LOG_BUFFER_SIZE, "%uGB", (uint32_t)(bytes / 1024.0 / 1024.0 / 1024.0));
    return LogBuff;
}

void logFS()
{
    if (!_logFS)
    {
        serial_println("No FS!");
        return;
    }
    size_t tb = 0;
    size_t ub = 0;
    GetFSBytes( tb, ub );
    logstr("FS: ");
    logstr(formatBytes(tb));
    logstr(" Used: ");
    logstr(formatBytes(ub));
    logln();
}

void logMem()
{
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "Free heap: %d", ESP.getFreeHeap());
    logln(LogBuff);
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "Heap size: %d", ESP.getHeapSize());
    logln(LogBuff);
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "PSRAM size: %d", ESP.getPsramSize());
    logln(LogBuff);
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "Free PSRAM: %d", ESP.getFreePsram());
    logln(LogBuff);
    if( !psramFound() )
    {
        logln("psramFound() - return false");
    }
    logln("Partition table:");
    logln("| Type | Sub |  Offset  |   Size   |       Label      |");
    logln("| ---- | --- | -------- | -------- | ---------------- |");
    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (pi != NULL) {
        do {
            const esp_partition_t* p = esp_partition_get(pi);
            LogBuff[0] = 0;
            snprintf(LogBuff, LOG_BUFFER_SIZE, "|  %02x  | %02x  | 0x%06X | 0x%06X | %-16s |", 
            p->type, p->subtype, p->address, p->size, p->label);
            logln(LogBuff);
            pi = esp_partition_next(pi);
        } while( pi != NULL);
    }
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "Sketch size: %u", ESP.getSketchSize());
    logln(LogBuff);
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "Free size  : %u", ESP.getFreeSketchSpace());
    logln(LogBuff);
}

//verify if file exist
bool checkFile(const char *path, size_t minSize) 
{
    if(!_logFS)
        return false;
    File f = _logFS->open(path, "r");
    if(!f)
        return false;
    bool cf = (f.size() >= minSize);
    f.close();
    return cf; 
}

