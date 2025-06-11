#pragma once

#ifndef _NVSImpl_h_20220614
#define _NVSImpl_h_20220614

#include <Arduino.h>
extern "C"
{
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
}

class NVSImpl
{
private:
    nvs_handle_t handle;
    bool flag;

    static bool SetErr(esp_err_t err, const char* name, bool do_log);
    static bool GetErr(esp_err_t err, const char* name, bool do_log);

public:
    bool doLog;
    static void Setup();

    NVSImpl(const char* name = "storage", nvs_open_mode_t open_mode = NVS_READWRITE, bool do_log = true);
    ~NVSImpl();

    bool set_u64(const char* name, uint64_t val);
    bool get_u64(const char* name, uint64_t& val);
    bool set_u32(const char* name, uint32_t val);
    bool get_u32(const char* name, uint32_t& val);
    bool set_u16(const char* name, uint16_t val);
    bool get_u16(const char* name, uint16_t& val);
    bool set_u8(const char* name, uint8_t val);
    bool get_u8(const char* name, uint8_t& val);

    bool set_i64(const char* name, int64_t val);
    bool get_i64(const char* name, int64_t& val);
    bool set_i32(const char* name, int32_t val);
    bool get_i32(const char* name, int32_t& val);
    bool set_i16(const char* name, int16_t val);
    bool get_i16(const char* name, int16_t& val);
    bool set_i8(const char* name, int8_t val);
    bool get_i8(const char* name, int8_t& val);

    bool set_str(const char* name, const String& val);
    bool get_str(const char* name, String& val);

    bool openOk() const { return flag; }
    bool erase(); // erase current key name
    static void Erase(const char* name);

    static void NVSLog();
};

#endif //_NVSImpl_h_20220614
