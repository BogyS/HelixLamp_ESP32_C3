// 1st. List (palettcrossfade): 1 effect
// 2nd. List (palette mode): 12 effects
// 3rd. List (effect mode): 11 effects
// 4th. List (solid color mode): 18 effects
#include <OneButton.h>

#include "hl_main.h"

#include "serial_config.h"
#include "led_config.h"
#include "mode.h"

#include "solid_color_mode.h"
#include "effect_mode.h"
#include "palette_mode.h"
#include "pacifica.h"
#include "pride.h"
#include "fire.h"
#include "twinkle.h"

#include "NVSImpl.h"
#include "esp_sleep.h"
#include "esp_system.h"

#include "Config.h"

#define BILED_PIN 8 // Built-in LED pin (on ESP32, it's GPIO 8)
#define BTN_PIN 2   // Button pin.

OneButton btn(BTN_PIN); // create button object that attach to pin 2

bool ledOn = false;
volatile bool isSleeping = false;
volatile bool isClicking = false;
volatile bool randomList = true;

Pacifica pacifica;
Pride pride;
Fire fire;
Test test;
TwinkleFox twinkle;
Effect* effectsMisc[] = {&pacifica, &pride, &fire, &test, &twinkle};

EffectList misc(effectsMisc, ARRAY_SIZE(effectsMisc));
Palette palette;
Efects efects;
SolidColor solidColor;

uint8_t listIdx = 0;
Mode* list[] = {&misc, &palette, &efects, &solidColor};

const __FlashStringHelper* listName(uint8_t idx)
{
    switch (idx)
    {
    case 0:
        return F("Misc");
    case 1:
        return F("Palette");
    case 2:
        return F("Effects");
    case 3:
        return F("Solid color");
    default:
        return F("?");
    }
}

const __FlashStringHelper* listName()
{
    return listName(listIdx);
}

void btnISR()
{
    btn.tick(); // Read pin and update FSM
}

void listPosInfo()
{
    DispV.pos_state = String("Effect: ");
    DispV.pos_state += list[listIdx]->Get();
    DispV.pos_state += " - ";
    DispV.pos_state += list[listIdx]->Name();

    serial_println(DispV.pos_state);
}

void listInfo()
{
    DispV.list_state = String("List: ");
    DispV.list_state += listIdx;
    DispV.list_state += " (";
    DispV.list_state += list[listIdx]->Max();
    DispV.list_state += " ) - ";
    DispV.list_state += String(listName());

    serial_println(DispV.list_state);
    listPosInfo();
}

void listInfoLeds()
{
    FastLED.clear(); // Set all LEDs to black (off)
    uint8_t i;
    for (i = 0; i <= listIdx; i++)
        leds[i % NUM_LEDS] = CRGB::Green;
    uint8_t m = list[listIdx]->Max();
    for (i = 0; i < m; i++)
        leds[(NUM_LEDS - 1 - i) % NUM_LEDS] = CRGB::Blue;
    FastLED.show(); // Apply the changes
    delay(2000);
    FastLED.clear(); // Set all LEDs to black (off)
    FastLED.show();  // Apply the changes
}

void listInfoPosLeds()
{
    FastLED.clear(); // Set all LEDs to black (off)
    uint8_t i;
    for (i = 0; i <= listIdx; i++)
        leds[i % NUM_LEDS] = CRGB::Green;
    uint8_t p = list[listIdx]->Get();
    uint8_t m = list[listIdx]->Max();
    for (i = 0; i < m; i++)
        leds[(NUM_LEDS - 1 - i) % NUM_LEDS] = (i == p) ? CRGB::Red : CRGB::Blue;
    FastLED.show(); // Apply the changes
    delay(1500);
    FastLED.clear(); // Set all LEDs to black (off)
    FastLED.show();  // Apply the changes
}

void listInfoRndLeds()
{
    FastLED.clear();
    leds[0] = CRGB::Green;
    leds[NUM_LEDS / 2] = CRGB::Blue;
    FastLED.show();
    delay(1000);
    FastLED.clear(); // Set all LEDs to black (off)
    FastLED.show();  // Apply the changes
}

void setRandomList()
{
    listIdx = random8(ARRAY_SIZE(list));
    list[listIdx]->Rnd();
    FastLED.setBrightness(BRIGHTNESS);
    listInfo();
    listInfoPosLeds();
    ledOn = !ledOn;
    digitalWrite(BILED_PIN, ledOn);
}

void resetRandomList()
{
    if (!randomList)
        return;
    randomList = false;
    for (uint8_t i = 0; i < ARRAY_SIZE(list); i++)
        list[i]->Init();
    listIdx = 0;
    FastLED.setBrightness(BRIGHTNESS);
    listInfo();
    ledOn = false;
    digitalWrite(BILED_PIN, ledOn);
}

// Handler function for button:
void hl_Click()
{
    if (isSleeping || isClicking)
        return; // do nothing in sleep mode
    isClicking = true;
    resetRandomList();
    serial_println(F("- Click!"));
    ledOn = !ledOn;
    digitalWrite(BILED_PIN, ledOn);
    list[listIdx]->Next();
    listPosInfo();
    listInfoPosLeds();
    isClicking = false;
}

void hl_DoubleClick()
{
    if (isSleeping || isClicking)
        return; // do nothing in sleep mode
    isClicking = true;
    resetRandomList();
    serial_println(F("- DoubleClick!"));
    listIdx = (listIdx + 1) % ARRAY_SIZE(list);
    FastLED.setBrightness(BRIGHTNESS);
    listInfo();
    listInfoLeds();
    list[listIdx]->Init();
    isClicking = false;
}

void hl_MultiClick()
{
    if (isSleeping || isClicking)
        return; // do nothing in sleep mode
    isClicking = true;
    serial_println(F("- MultiClick!"));
    randomList = true; // start random
    listInfoRndLeds();
    isClicking = false;
}

void hl_LongPressStart()
{
    if (isSleeping || isClicking)
        return; // do nothing in sleep mode
                // Disable the current interrupt
    detachInterrupt(digitalPinToInterrupt(BTN_PIN));
    isClicking = true;
    isSleeping = true;
    serial_println(F("- LongPressStart!"));
    serial_println(F("Entering sleep mode..."));
    FastLED.clear();
    FastLED.show();
    delay(500);
    ledOn = false;
    digitalWrite(BILED_PIN, ledOn);
    // save data
    // EEPROM.update(0, randomList ? 1 : 0); // random
    // EEPROM.update(1, listIdx);
    // EEPROM.update(2, list[listIdx]->Get());
    // save data to NVS
    NVSImpl nvs("hl_data", NVS_READWRITE);
    nvs.set_u8("rndLst", randomList ? 1 : 0);
    nvs.set_u8("lstIdx", listIdx);
    nvs.set_u8("lstPos", list[listIdx]->Get());

    // Configure GPIO wake-up on button press (LOW level)
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 0);
    esp_deep_sleep_enable_gpio_wakeup((1ULL << BTN_PIN), ESP_GPIO_WAKEUP_GPIO_LOW);

    // Start deep sleep (device will reboot on wake-up)
    esp_deep_sleep_start();
    //... The device will not return from this function
    isClicking = false;
}

void hl_Sleep()
{
    hl_LongPressStart(); // Call the long press handler to enter sleep mode
    // Note: The device will not return from this function after deep sleep
}

uint8_t hl_NoLists()
{
    return (uint8_t)ARRAY_SIZE(list);
}

String hl_ListName(uint8_t idx)
{
    return String(listName(idx));
}

uint8_t hl_ListMax(uint8_t idx)
{
    if (idx >= ARRAY_SIZE(list))
        return 0;
    return list[idx]->Max();
}

String hl_EffectName(uint8_t l_idx, uint8_t e_idx)
{
    if (l_idx >= ARRAY_SIZE(list))
        return String(F("Unknown"));
    if (e_idx >= list[l_idx]->Max())
        return String(F("Unknown"));
    return String(list[l_idx]->Name(e_idx));
}

void hl_Get(uint8_t& l_idx, uint8_t& e_idx, bool& isRandom)
{
    l_idx = listIdx;
    e_idx = list[listIdx]->Get();
    isRandom = randomList;
}

void hl_Set(uint8_t l_idx, uint8_t e_idx, bool isRandom)
{
    if (l_idx >= ARRAY_SIZE(list))
        return;
    listIdx = l_idx;
    if (e_idx >= list[listIdx]->Max())
        e_idx = 0; // reset to 0
    list[listIdx]->Set(e_idx);
    randomList = isRandom;
    FastLED.setBrightness(BRIGHTNESS);
    listInfo();
    listInfoLeds();
    ledOn = !ledOn;
    digitalWrite(BILED_PIN, ledOn);
}

void hl_setup()
{
    pinMode(BILED_PIN, OUTPUT);
    // pin mode i
    // pinMode(BTN_PIN, INPUT_PULLUP);
    // attaching a interrupt to pin D2
    attachInterrupt(digitalPinToInterrupt(BTN_PIN), btnISR, CHANGE);

    btn.attachClick(hl_Click);
    btn.attachDoubleClick(hl_DoubleClick);
    btn.attachMultiClick(hl_MultiClick);
    btn.setClickMs(500);
    btn.attachLongPressStart(hl_LongPressStart);

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();

    for (uint8_t i = 0; i < ARRAY_SIZE(list); i++)
        list[i]->Init();

    // read data
    uint8_t val;
    NVSImpl nvs("hl_data", NVS_READONLY);
    if (nvs.get_u8("rndLst", val))
        randomList = (val == 1);
    if (nvs.get_u8("lstIdx", val))
        listIdx = val % ARRAY_SIZE(list);
    if (nvs.get_u8("lstPos", val))
        list[listIdx]->Set(val);

    listInfo();
    listInfoLeds();
    if (randomList)
        listInfoRndLeds();
}

void hl_loop()
{
    if (isSleeping)
    {
        serial_println(F("Wake info:"));
        listInfo();
        isSleeping = false;
    }
    btn.tick();
    if (randomList)
    {
        EVERY_N_MILLISECONDS(7000)
        {
            setRandomList();
        }
    }
    list[listIdx]->Update();
}

uint8_t progressSteps = 0;
uint8_t progressPos = 0;

void hl_progressStart(uint8_t steps)
{
    progressSteps = steps;
    progressPos = 0;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void hl_progressStep()
{
    if (progressSteps == 0)
        return; // avoid divide-by-zero

    progressPos++;
    uint8_t numLit = map(progressPos, 0, progressSteps, 0, NUM_LEDS);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (uint8_t i = 0; i < numLit; i++)
    {
        leds[i] = CRGB::Green;
    }
    FastLED.show();
}

void hl_progressStop()
{
    fill_solid(leds, NUM_LEDS, CRGB::Blue); // Or any "ready" color
    FastLED.show();
    delay(1000);
    listInfoPosLeds();
}
