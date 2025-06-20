#pragma once

#ifndef EFFECT_MODE_H_
#define EFFECT_MODE_H_

#include "led_config.h"
#include "mode.h"

namespace EffectMode
{
    uint8_t gHue = 0; // rotating "base color" used by many of the patterns

    //////////////////////////
    void rainbow()
    {
        // FastLED's built-in rainbow generator
        fill_rainbow(leds, NUM_LEDS, gHue, 12);
    }

    //////////////////////////
    void confetti()
    {
        // random colored speckles that blink in and fade smoothly
        fadeToBlackBy(leds, NUM_LEDS, 20);
        int pos = random16(NUM_LEDS);
        leds[pos] += CHSV(gHue + random8(64), random8(128, 200), random8(48, 255));
    }

    //////////////////////////
    void confetti_GB()
    {
        // random colored speckles, Green and Blue hues only
        // Green = 96, Blue = 160
        uint8_t p = 25; // What percentage of the time to make speckles.  [Range 0-100]

        fadeToBlackBy(leds, NUM_LEDS, 10);
        if (random8(100) < p)
        {
            int pos = random16(NUM_LEDS);
            uint8_t hue = random8(2); // randomly chooses a 0 or 1
            if (hue == 0)
            {
                hue = random8(92, 111); // pick a hue somewhere in the green zone
            }
            else
            {
                hue = random8(156, 165); // pick a hue somewhere in the blue zone
            }
            leds[pos] += CHSV(hue, random8(200, 240), 255);
        }
    } // end confetti_GB

    //////////////////////////
    void sinelon()
    {
        // a colored dot sweeping back and forth, with fading trails
        fadeToBlackBy(leds, NUM_LEDS, 12);
        int pos = beatsin16(13, 0, NUM_LEDS - 1);
        leds[pos] += CHSV(gHue, 255, 192);
    }

    //////////////////////////
    void juggle()
    {
        // four colored dots, weaving in and out of sync with each other
        fadeToBlackBy(leds, NUM_LEDS, 20);
        byte dothue = 0;
        for (int i = 0; i < 4; i++)
        {
            leds[beatsin16(i + 5, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }
    }

    //////////////////////////
    void spew()
    {
        const uint16_t spewSpeed = 100; // rate of advance
        static boolean spewing = 0;     // pixels are On(1) or Off(0)
        static uint8_t count = 1;       // how many to light (or not light)
        static uint8_t temp = count;
        static uint8_t hue = random8();
        EVERY_N_MILLISECONDS(spewSpeed)
        {
            if (count == 0)
            {
                spewing = !spewing;
                if (spewing == 1)
                {
                    count = random8(2, 5);
                } // random number for On pixels
                else
                {
                    count = random8(1, 8);
                } // random number for Off pixels
                temp = count;
                // gHue = gHue - 30;
                hue = random8();
            }
            for (uint8_t i = NUM_LEDS - 1; i > 0; i--)
            {
                leds[i] = leds[i - 1]; // shift data down the line by one pixel
            }
            if (spewing == 1)
            { // new pixels are On
                if (temp == count)
                {
                    leds[0] = CHSV(hue - 5, 215, 255); // for first dot
                                                       // leds[0] = CHSV(gHue-5, 215, 255);  // for first dot
                }
                else
                {
                    leds[0] = CHSV(hue, 255, 255 / (1 + ((temp - count) * 2))); // for following dots
                                                                                // leds[0] = CHSV(gHue, 255, 255/(1+((temp-count)*2)) );  // for following dots
                }
            }
            else
            {                            // new pixels are Off
                leds[0] = CHSV(0, 0, 0); // set pixel 0 to black
            }
            count = count - 1; // reduce count by one.
        } // end every_n
    } // end spew

    //////////////////////////
    void spewFour()
    {
        // Similar to the abouve "spew", but split up into four sections,
        // specifically designed for a 8x4 matrix with Z-layout.
        const uint16_t spewSpeed = 100;           // rate of advance
        static uint8_t spewing[4] = {0, 0, 0, 0}; // pixels are On(1) or Off(0)
        static uint8_t count[4] = {1, 1, 1, 1};   // how many to light (or not light)
        static uint8_t temp[4] = {count[0], count[1], count[2], count[3]};
        static uint8_t hue[4] = {random8(), random8(), random8(), random8()};
        EVERY_N_MILLISECONDS(spewSpeed)
        {
            for (uint8_t j = 0; j < 4; j++)
            {
                if (count[j] == 0)
                {
                    if (spewing[j] == 0)
                    {
                        spewing[j] = 1;
                    }
                    else
                    {
                        spewing[j] = 0;
                    }
                    if (spewing[j] == 1)
                    {
                        count[j] = random8(2, 5);
                    } // random number for On pixels
                    else
                    {
                        count[j] = random8(1, 8);
                    } // random number for Off pixels
                    temp[j] = count[j];
                    EVERY_N_SECONDS(2)
                    { // hue going across is constant for awhile
                        hue[j] = random8();
                    }
                }
                for (uint8_t i = 7; i > 0; i--)
                {
                    leds[(j * 8) + i] = leds[(j * 8) + i - 1]; // shift data down the line by one pixel
                }
                if (spewing[j] == 1)
                { // new pixels are On
                    if (temp[j] == count[j])
                    {
                        leds[((j + 1) * 8) - 8] = CHSV(hue[j] - 5, 215, 255); // for first dot
                    }
                    else
                    {
                        leds[((j + 1) * 8) - 8] = CHSV(hue[j], 255, 255 / (1 + ((temp[j] - count[j]) * 2))); // for following dots
                    }
                }
                else
                {                                            // new pixels are Off
                    leds[((j + 1) * 8) - 8] = CHSV(0, 0, 0); // set pixel 0 to black
                }
                count[j] = count[j] - 1; // reduce count by one.
            } // end for loop
        } // end every_n
    } // end spewFour

    //////////////////////////
    void blinkyblink1()
    {
        static boolean dataIncoming = LOW;
        static boolean blinkGate1 = LOW;
        static boolean blinkGate2 = HIGH;
        static int8_t count = -1;

        EVERY_N_MILLISECONDS_I(timingObj, 250)
        {
            count++;
            if (count == 6)
            {
                count = 0;
            }
            blinkGate2 = count;
            dataIncoming = !dataIncoming;
            blinkGate1 = !blinkGate1;
            FastLED.clear();
            leds[0] = CHSV(gHue, 0, dataIncoming * blinkGate1 * 255 * blinkGate2);
            if (count == 2 || count == 3)
            {
                timingObj.setPeriod(50);
            }
            else if (count == 4)
            {
                timingObj.setPeriod(405);
            }
            else
            {
                timingObj.setPeriod(165);
            }
        }
    } // end_blinkyblink1

    //////////////////////////
    void blinkyblink2()
    {
        static boolean dataIncoming = LOW;
        static boolean blinkGate1 = LOW;
        static boolean blinkGate2 = HIGH;
        static int8_t count = -1;
        static int8_t P;

        EVERY_N_MILLISECONDS_I(timingObj, 250)
        {
            count++;
            if (count == 8)
            {
                count = 0;
                P = random8(NUM_LEDS);
            }
            blinkGate2 = count;
            dataIncoming = !dataIncoming;
            blinkGate1 = !blinkGate1;
            FastLED.clear();
            leds[P] = CHSV(gHue, 255, dataIncoming * blinkGate1 * 255 * blinkGate2);
            if (count == 6)
            {
                timingObj.setPeriod(250);
            }
            else if (count == 7)
            {
                timingObj.setPeriod(500);
            }
            else
            {
                timingObj.setPeriod(25);
            }
        }
    } // end_blinkyblink2

    //////////////////////////
    void fillAndCC()
    {
        static int16_t pos = 0;  // position along strip
        static int8_t delta = 3; // delta (can be negative, and/or odd numbers)
        static uint8_t hue = 0;  // hue to display
        EVERY_N_MILLISECONDS(50)
        {
            leds[pos] = CHSV(hue, 255, 255);
            pos = (pos + delta + NUM_LEDS) % NUM_LEDS;
            if (delta >= 0 && pos == 0)
            { // going forward
                hue = hue + random8(42, 128);
            }
            if (delta < 0 && pos == NUM_LEDS - 1)
            { // going backward
                hue = hue + random8(42, 128);
            }
        }
    } // fillAndCC

    //////////////////////////
    void applause() // originally by Mark K.
    {
        // Number of loops is increased based on number of pixels.
        // Fade ammount drops as pixels increase.
        static uint8_t appAmmount = (NUM_LEDS / 4);
        for (uint8_t i = 0; i < appAmmount; i++)
        {
            static uint16_t lastPixel = 0;
            fadeToBlackBy(leds, NUM_LEDS, 12); // was 32
            leds[lastPixel] = CHSV(random8(HUE_BLUE, HUE_PURPLE), 255, 255);
            lastPixel = random16(NUM_LEDS);
            leds[lastPixel] = CRGB::White;
        }
    }

    //////////////////////////
    void twoDots()
    {
        static uint8_t pos; // used to keep track of position
        EVERY_N_MILLISECONDS(70)
        {
            fadeToBlackBy(leds, NUM_LEDS, 200); // fade all the pixels some
            leds[pos] = CHSV(gHue, random8(170, 230), 255);
            leds[(pos + 5) % NUM_LEDS] = CHSV(gHue + 64, random8(170, 230), 255);
            pos = pos + 1; // advance position

            // This following check is very important.  Do not go past the last pixel!
            if (pos == NUM_LEDS)
            {
                pos = 0;
            } // reset to beginning
            // Trying to write data to non-existent pixels causes bad things.
        }
    } // end_twoDots

    fnPtrVoid efectsArray[] = {twoDots, fillAndCC, blinkyblink2, spewFour, spew, confetti_GB, rainbow, applause, confetti, sinelon, juggle};
    // const char* efectsNames[] = {"Two Dots", "Fill And CC", "Bblinky blink", "Spew Four", "Spew", "Confetti GB", "Rainbow", "Applause", "Confetti", "Sinelon", "Juggle"};
} // EffectMode

class Efects : public Mode
{
public:
    void Update()
    {
        EffectMode::efectsArray[counter]();
        FastLED.show();
        FastLED.delay(2000 / UPDATES_PER_SECOND);
        EVERY_N_MILLISECONDS(20)
        {
            EffectMode::gHue++;
        }
    }
    uint8_t Max() const
    {
        return ARRAY_SIZE(EffectMode::efectsArray);
    }
    const __FlashStringHelper* Name(uint8_t idx) const
    {
        switch (idx)
        {
        case 0:
            return F("Two Dots");
        case 1:
            return F("Fill And CC");
        case 2:
            return F("Bblinky blink");
        case 3:
            return F("Spew Four");
        case 4:
            return F("Spew");
        case 5:
            return F("Confetti GB");
        case 6:
            return F("Rainbow");
        case 7:
            return F("Applause");
        case 8:
            return F("Confetti");
        case 9:
            return F("Sinelon");
        case 10:
            return F("Juggle");
        default:
            return F("Unknown Effect");
        }
    }
};

#endif /* EFFECT_MODE_H_ */
