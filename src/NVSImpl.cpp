#include "NVSImpl.h"
#include "FSLog.h"

// static
void NVSImpl::Setup()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    // ESP_ERROR_CHECK( err );
}

NVSImpl::NVSImpl(const char* name, nvs_open_mode_t open_mode, bool do_log)
{
    doLog = do_log;
    flag = true;
    esp_err_t err = nvs_open(name, open_mode, &handle);
    if (err != ESP_OK)
    {
        flag = false;
        if (doLog)
        {
            LogBuff[0] = 0;
            snprintf(LogBuff, LOG_BUFFER_SIZE, "Error (%s) open NVS (%s)!",
                     esp_err_to_name(err), name);
            logln(LogBuff);
        }
    }
}

NVSImpl::~NVSImpl()
{
    if (flag)
        nvs_close(handle);
}

// static
bool NVSImpl::SetErr(esp_err_t err, const char* name, bool do_log)
{
    if (err == ESP_OK)
        return true;
    if (!do_log)
        return false;
    logstr("Variable: ");
    logstr(name);
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, " - Error (%s) on set!", esp_err_to_name(err));
    logln(LogBuff);
    return false;
}
// static
bool NVSImpl::GetErr(esp_err_t err, const char* name, bool do_log)
{
    if (err == ESP_OK)
        return true;
    if (!do_log)
        return false;
    logstr("Variable: ");
    logstr(name);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        logln(" - is not initialized yet!");
    }
    else
    {
        LogBuff[0] = 0;
        snprintf(LogBuff, LOG_BUFFER_SIZE, " - Error (%s) on get!", esp_err_to_name(err));
        logln(LogBuff);
    }
    return false;
}

bool NVSImpl::set_u64(const char* name, uint64_t val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_u64(handle, name, val);
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}

bool NVSImpl::get_u64(const char* name, uint64_t& val)
{
    if (flag)
    {
        uint64_t ui64 = 0;
        esp_err_t err = nvs_get_u64(handle, name, &ui64);
        if (GetErr(err, name, doLog))
        {
            val = ui64;
            return true;
        }
    }
    return false;
}

bool NVSImpl::set_u32(const char* name, uint32_t val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_u32(handle, name, val);
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}

bool NVSImpl::get_u32(const char* name, uint32_t& val)
{
    if (flag)
    {
        uint32_t ui32 = 0;
        esp_err_t err = nvs_get_u32(handle, name, &ui32);
        if (GetErr(err, name, doLog))
        {
            val = ui32;
            return true;
        }
    }
    return false;
}

bool NVSImpl::set_u16(const char* name, uint16_t val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_u16(handle, name, val);
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}

bool NVSImpl::get_u16(const char* name, uint16_t& val)
{
    if (flag)
    {
        uint16_t ui16 = 0;
        esp_err_t err = nvs_get_u16(handle, name, &ui16);
        if (GetErr(err, name, doLog))
        {
            val = ui16;
            return true;
        }
    }
    return false;
}

bool NVSImpl::set_u8(const char* name, uint8_t val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_u8(handle, name, val);
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}

bool NVSImpl::get_u8(const char* name, uint8_t& val)
{
    if (flag)
    {
        uint8_t ui8 = 0;
        esp_err_t err = nvs_get_u8(handle, name, &ui8);
        if (GetErr(err, name, doLog))
        {
            val = ui8;
            return true;
        }
    }
    return false;
}

bool NVSImpl::set_i64(const char* name, int64_t val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_i64(handle, name, val);
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}

bool NVSImpl::get_i64(const char* name, int64_t& val)
{
    if (flag)
    {
        int64_t i64 = 0;
        esp_err_t err = nvs_get_i64(handle, name, &i64);
        if (GetErr(err, name, doLog))
        {
            val = i64;
            return true;
        }
    }
    return false;
}

bool NVSImpl::set_i32(const char* name, int32_t val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_i32(handle, name, val);
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}

bool NVSImpl::get_i32(const char* name, int32_t& val)
{
    if (flag)
    {
        int32_t i32 = 0;
        esp_err_t err = nvs_get_i32(handle, name, &i32);
        if (GetErr(err, name, doLog))
        {
            val = i32;
            return true;
        }
    }
    return false;
}

bool NVSImpl::set_i16(const char* name, int16_t val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_i16(handle, name, val);
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}

bool NVSImpl::get_i16(const char* name, int16_t& val)
{
    if (flag)
    {
        int16_t i16 = 0;
        esp_err_t err = nvs_get_i16(handle, name, &i16);
        if (GetErr(err, name, doLog))
        {
            val = i16;
            return true;
        }
    }
    return false;
}

bool NVSImpl::set_i8(const char* name, int8_t val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_i8(handle, name, val);
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}

bool NVSImpl::get_i8(const char* name, int8_t& val)
{
    if (flag)
    {
        int8_t i8 = 0;
        esp_err_t err = nvs_get_i8(handle, name, &i8);
        if (GetErr(err, name, doLog))
        {
            val = i8;
            return true;
        }
    }
    return false;
}

bool NVSImpl::set_str(const char* name, const String& val)
{
    if (flag)
    {
        esp_err_t err = nvs_set_str(handle, name, val.c_str());
        if (SetErr(err, name, doLog))
        {
            nvs_commit(handle);
            return true;
        }
    }
    return false;
}
bool NVSImpl::get_str(const char* name, String& val)
{
    if (!flag)
        return false;
    size_t strSize = 0;
    esp_err_t err = nvs_get_str(handle, name, NULL, &strSize);
    if (err == ESP_OK)
    {
        if (strSize < LOG_BUFFER_SIZE)
        { // use log buff
            strSize = LOG_BUFFER_SIZE;
            err = nvs_get_str(handle, name, LogBuff, &strSize);
            if (err == ESP_OK)
                val = String(LogBuff);
        }
        else
        {
            char* out_value = (char*)malloc(strSize);
            err = nvs_get_str(handle, name, out_value, &strSize);
            if (err == ESP_OK)
                val = String(out_value);
            free(out_value);
        }
    }
    return GetErr(err, name, doLog);
}

// erase current key name
bool NVSImpl::erase()
{
    if (!flag)
        return false;
    esp_err_t err = nvs_erase_all(handle);
    if (err != ESP_OK)
        return false;
    err = nvs_commit(handle);
    return (err == ESP_OK);
}

void NVSImpl::Erase(const char* name)
{
    NVSImpl nvsk(name, NVS_READWRITE, false);
    nvsk.erase();
}

void NVSImpl::NVSLog()
{
    nvs_stats_t nvs_stats;
    nvs_get_stats(NULL, &nvs_stats);
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "NVS - Count: %d", nvs_stats.namespace_count);
    logln(LogBuff);
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "Entries: Used = (%d), Free = (%d), All = (%d)",
             nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries);
    logln(LogBuff);
}