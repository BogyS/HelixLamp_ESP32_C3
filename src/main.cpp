#include "Config.h"
#include "serial_config.h"
#include <esp-fs-webserver.h>
// https://github.com/cotestatnt/esp-fs-webserver
// https://github.com/BogyS/esp-fs-webserver
#include <time.h>

#include "FSDef.h"
#include "FSLog.h"
#include "FAux.h"
#include "NVSImpl.h"
#include "WebCmd.h"
#include "hl_main.h"

const char Name[] = "Helix Lamp";
const char Ver[] = "25.06.11.09"; // version of firmware
const char MDNSName[] = "helix";  // used for mDNS and hostname
// http://helix.local/...

FSWebServer myWebServer(FILESYSTEM, 80, MDNSName);

WebSocketsServer webSocket(81);
bool broadcastTXT(const char* payload, size_t length)
{
    return webSocket.broadcastTXT((uint8_t*)payload, length);
}
bool broadcastTXT(String& payload)
{
    return webSocket.broadcastTXT((uint8_t*)payload.c_str(), payload.length());
}
bool broadcastTXT(const char* pStr)
{
    return webSocket.broadcastTXT((uint8_t*)pStr, strlen(pStr));
}

uint32_t getChipId()
{
    uint32_t id = 0;
    uint64_t macAddress = ESP.getEfuseMac();
    for (int i = 0; i < 17; i = i + 8)
    {
        id |= ((macAddress >> (40 - i)) & 0xff) << i;
    }
    return id;
}

void handleInfo()
{
    String r = "v";
    r += Ver;
    r += "<br>Cip ID: ";
    r += String(getChipId(), HEX);
    r += "<br>IP: ";
    r += DispV.ip;
    r += "<br>RRSI: ";
    r += DispV.rrsi;
    r += "<br>Time: ";
    r += DispV.time;
    r += "<br>";
    r += DispV.list_state;
    r += "<br>";
    r += DispV.pos_state;
    myWebServer.send(200, "text/plain", r.c_str());
}

void handleClock()
{
    time_t t;
    time(&t); // current time
    struct tm timeinfo;
    localtime_r(&t, &timeinfo);
    LogBuff[0] = 0;
    strftime(LogBuff, LOG_BUFFER_SIZE, "%A, %B %d %Y %H:%M:%S", &timeinfo);
    serial_print("Handle Clock: ");
    serial_println(LogBuff);
    myWebServer.send(200, "text/plain", LogBuff);
}

void DispWiFi()
{
    time_t t;
    time(&t);
    logBuffHMS(t);
    DispV.time = LogBuff;
    DispV.ip = WiFi.localIP().toString();
    LogBuff[0] = 0;
    int8_t rssi = WiFi.RSSI();
    snprintf(LogBuff, LOG_BUFFER_SIZE, "RRSI: %d", rssi);
    DispV.rrsi = LogBuff;
}

void disconnectImpl()
{
    logln("Disconnect!");
    WiFi.disconnect();
    myWebServer.clearWifiCredentials();
    IPAddress ip = myWebServer.startAP();
    String str("WiFi connected: ");
    str += ip.toString();
    logln(str.c_str());
    DispWiFi();
}

void readVal(JsonDocument& doc, const char* key, String& str)
{
    const char* strVal = doc[key];
    if (strVal)
        str = strVal;
}

template <typename T>
void readVal(JsonDocument& doc, const char* key, T& val)
{
    if (!doc[key].isNull())
        val = doc[key].as<T>();
}
//-------
JsonDocument CfgDoc;
bool CfgLoad = false; // save config

#define CfgVal(val)                 \
    if (CfgLoad)                    \
        readVal(CfgDoc, #val, val); \
    else                            \
        CfgDoc[#val] = val;

//-------
void configValues()
{
    // Clock
    CfgVal(NTPServer);
    CfgVal(GMTOffsetSec);
    CfgVal(DaylightOffsetSec);
}

void initConfig()
{
    myWebServer.addOption("name-logo", Name);
    myWebServer.setFirmwareVersion(Ver);
    myWebServer.closeConfiguration();
}

bool loadConfig()
{
    constexpr const char* fn = "/setup/helix_lamp.json";
    logln("Load config ...");
    File file = FILESYSTEM.open(fn, "r");
    if (!file)
    {
        logstr("Failed to open: ");
        logln(fn);
        return false;
    }
    CfgDoc.clear();
    DeserializationError error = deserializeJson(CfgDoc, file);
    file.close();
    if (error)
    {
        logln("Failed to deserialize JSON!");
        logln(error.c_str());
        return false;
    }
    CfgLoad = true;
    configValues();
    CfgDoc.clear();
    logln("Ok.");
    return true;
}

void factoryReset()
{
    logln("Factory reset!");
    ESP_ERROR_CHECK(nvs_flash_erase()); // erase default partition
    ESP_ERROR_CHECK(nvs_flash_init());  // re-init
    logln("Delete all credentials:");
    disconnectImpl();
}

void getFSInfo(fsInfo_t* info)
{
    if (!info)
        return;
    GetFSBytes(info->totalBytes, info->usedBytes);
}

void handleEffects()
{
    JsonDocument doc;
    doc.clear();

    JsonArray lists = doc["lists"].to<JsonArray>();

    uint8_t noLists = hl_NoLists();
    for (uint8_t i = 0; i < noLists; ++i)
    {
        JsonObject l = lists.add<JsonObject>();
        l["name"] = hl_ListName(i);

        JsonArray effects = l["effects"].to<JsonArray>();
        uint8_t maxEffects = hl_ListMax(i);
        for (uint8_t j = 0; j < maxEffects; ++j)
        {
            effects.add(hl_EffectName(i, j));
        }
    }

    String json;
    serializeJson(doc, json);
    myWebServer.send(200, "application/json", json);
}

void handleGetEffect()
{
    uint8_t l_idx = 0;
    uint8_t e_idx = 0;
    bool isRandom = false;
    hl_Get(l_idx, e_idx, isRandom);

    JsonDocument doc;
    doc["list_idx"] = l_idx;
    doc["effect_idx"] = e_idx;
    doc["random"] = isRandom ? 1 : 0;

    String json;
    serializeJson(doc, json);
    myWebServer.send(200, "application/json", json);
}

void handleSetEffect()
{
    if (!myWebServer.hasArg("plain"))
    {
        myWebServer.send(400, "text/plain", "Missing body");
        return;
    }

    String body = myWebServer.arg("plain");

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err)
    {
        myWebServer.send(400, "text/plain", "Invalid JSON");
        return;
    }

    logln("Set effect:");
    logln(body.c_str());

    if (!doc["list_idx"].isNull() && !doc["effect_idx"].isNull() && !doc["random"].isNull())
    {
        uint8_t l_idx = doc["list_idx"].as<int>();
        uint8_t e_idx = doc["effect_idx"].as<int>();
        bool isRandom = doc["random"].as<int>() != 0; // Convert to bool

        logstr("List index: ");
        logln(String(l_idx).c_str());
        logstr("Effect index: ");
        logln(String(e_idx).c_str());
        logstr("Is random: ");
        logln(isRandom ? "true" : "false");

        hl_Set(l_idx, e_idx, isRandom);

        String msg = "Effect changed: ";
        msg += hl_ListName(l_idx);
        msg += " - ";
        msg += hl_EffectName(l_idx, e_idx);

        broadcastTXT(msg.c_str());
        logln(msg.c_str());

        myWebServer.send(200, "text/plain", "OK");
    }
    else
    {
        myWebServer.send(400, "text/plain", "Invalid or missing fields");
    }
}

void handleSleep()
{
    myWebServer.send(200, "text/plain", "OK");

    logValidTime();
    logln("Sleep ...");
    broadcastTXT("Sleep ...");
    hl_Sleep(); // go to sleep
}

void setup()
{
    serial_begin(115200);
    serial_println();
    if (!StartFS(true))
        ESP.restart();
    NVSImpl::Setup();
    setupLog(FILESYSTEM);
    logln("--------------------");
    LogBuff[0] = 0;
    snprintf(LogBuff, LOG_BUFFER_SIZE, "%s v%s", Name, Ver);
    logln(LogBuff);

    // start effects as soon as possible
    logln("Setup effects ...");
    hl_setup(); // setup the effects

    hl_progressStart(6);
    hl_progressStep();  //1

    String apName = MDNSName;
    apName += "-";
    apName += String(getChipId(), HEX);
    myWebServer.setAP(apName.c_str(), "lamp");
    // Try to connect to flash stored SSID
    // start AP if fails after timeout
    IPAddress myIP = myWebServer.startWiFi(5000, true);
    logln("WiFi started");
    logln("IP address: ");
    logln(myIP.toString().c_str());
    DispWiFi();

    hl_progressStep();  //2

    initConfig();

    myWebServer.on("/clock", handleClock);
    myWebServer.on("/info", handleInfo);
    myWebServer.on("/effects", HTTP_GET, handleEffects);
    myWebServer.on("/get_effect", HTTP_GET, handleGetEffect);
    myWebServer.on("/set_effect", HTTP_POST, handleSetEffect);
    myWebServer.on("/sleep", HTTP_POST, handleSleep);

    myWebServer.begin();
    myWebServer.enableFsCodeEditor(getFSInfo);
    serial_print(F("ESP Web Server started on IP Address: "));
    serial_println(myIP);
    serial_println(F("Open /setup page to configure optional parameters"));
    serial_println(F("Open /edit page to view and edit files"));
    logln("HTTP server started");

    hl_progressStep();  //3

    webSocket.begin();
    logln("WebSocket server started");

    hl_progressStep();  //4

    if (loadConfig())
    {
        serial_println(F("Application option loaded"));
    }
    else
    {
        serial_println(F("Application NOT loaded!"));
        serial_print(F("Open http://"));
        serial_print(myIP);
        serial_println(F("/setup to configure parameters"));
    }
    /*
    if (MDNS.begin(MDNSName))
    {
        logln("MDNS started. Address:");
        LogBuff[0] = 0;
        snprintf(LogBuff, LOG_BUFFER_SIZE, "http:/%s.local/", MDNSName);
        logln(LogBuff);
        MDNS.addService("http", "tcp", 80);
    }
    */
    logFS();
    logMem();

    initAux();
    loadAux();
    setupAux();

    hl_progressStep();  //5

    // init and get the time
    configTime(GMTOffsetSec, DaylightOffsetSec, NTPServer.c_str());
    logstr("Local time: ");
    logLocalTime(); // wait for time
    // after wifi an time set
    SaveTime.setup(true);   // save from now
    UpdateTime.setup(true); // update from now

    NVSImpl::NVSLog();
    logln("**********");

    hl_progressStep();  //6
    delay(100);
    hl_progressStop();
}

wl_status_t WiFiStatus = WL_IDLE_STATUS;

#define WIFI_TIMEOUT 5000 // 5 seconds
unsigned long lastWifiBegin = 0;
#define WIFI_RRSI_TIMEOUT 10000 // 10 seconds
unsigned long lastWifiRRSI = 0;

void loop()
{
    wl_status_t wls = WiFi.status();
    if (WiFiStatus != wls)
    {
        logHMS();
        if (wls == WL_CONNECTED)
        {
            // Print local IP address and start web server
            String str("WiFi connected: ");
            str += WiFi.localIP().toString();
            logln(str.c_str());
        }
        else
        {
            logln("WiFi NOT connected!");
            if (millis() - lastWifiBegin > WIFI_TIMEOUT)
            {
                // try to reconnect
                WiFi.reconnect();
                lastWifiBegin = millis();
            }
        }
    }
    if ((wls == WL_CONNECTED) && !localTimeOK)
        logValidTime();
    WiFiStatus = wls;
    if (millis() - lastWifiRRSI > WIFI_RRSI_TIMEOUT)
    {
        DispWiFi();
        lastWifiRRSI = millis();
        // updateScreen();
    }
    myWebServer.run();
    webSocket.loop();
    // allarms
    loopAux();
    // handle effects
    hl_loop();
    // Allow CPU to switch to other tasks.
    delay(2);
}