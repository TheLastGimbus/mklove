#ifndef MKLOVE_LEDUTILS_H
#define MKLOVE_LEDUTILS_H

#include <FastLED.h>

#define NUM_LEDS 10

namespace ledutils {
    typedef enum {
        TransBlue = 0x000aff,
        TransPink = 0xff0a82,
        Orange = 0xff3300,
        Pink = 0xff1010,
        Purple = 0x6600ff,
    } Colors;

    void flag(CRGB *leds, CRGB firstRow, CRGB secondRow, CRGB thirdRow, CRGB fourthRow, CRGB fifthRow) {
        leds[6] = leds[4] = firstRow;
        leds[7] = leds[5] = leds[3] = secondRow;
        leds[2] = leds[8] = thirdRow;
        leds[1] = leds[9] = fourthRow;
        leds[0] = fifthRow;
    }

    void mirror(CRGB *leds, CRGB *sixColors) {
        leds[0] = sixColors[0];
        leds[1] = leds[9] = sixColors[1];
        leds[2] = leds[8] = sixColors[2];
        leds[3] = leds[7] = sixColors[3];
        leds[4] = leds[6] = sixColors[4];
        leds[5] = sixColors[5];
    }

    void transFlag(CRGB *leds) { flag(leds, TransBlue, TransPink, CRGB::White, TransPink, TransBlue); }

    // YELLOW, WHITE, VIOLET, BLACK
    // Ugly as fuck, do not use
    void nonbinaryFlag(CRGB *leds) {
        flag(leds, CRGB::Black, CRGB::Yellow, CRGB::White, CRGB::Violet, CRGB::Black);
    }

    // PURPLE, WHITE, GREEN
    void genderqueerFlag(CRGB *leds) {
        flag(leds, CRGB::Purple, CRGB::Purple, CRGB::White, CRGB::Green, CRGB::Green);
    }

    // RED, ORANGE, YELLOW, GREEN, BLUE
    // Not used since we already have rainbow below
    void lgbtFlag(CRGB *leds) {
        flag(leds, CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue);
    }

    /// Fills leds array with rainbow colors, with rotation set by progress
    void rainbow(CRGB *leds, uint8_t numLeds, uint8_t progress) {
        for (int i = 0; i < numLeds; i++) leds[i] = CHSV(i * 255 / numLeds + progress, 255, 255);
    }

    /// Blinks in rhythm of heartbeat (XoXoooooooXoXooooooo.. you get it)
    void redBlink(CRGB *leds, uint8_t numLeds, uint8_t progress) {
        for (int i = 0; i < numLeds; i++) {
            leds[i] = (progress < 30 || (progress > 60 && progress < 90)) ? CRGB::Red : CRGB::Black;
        }
    }

    void batteryIndicator(CRGB *leds, uint8_t batteryLevel) {
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = i <= batteryLevel ? CRGB::Red : CRGB::Black;
    }

    // This is cool, but requires stable batteryLevel (some median done) and uses *precious memory*
    void batteryIndicatorAnimation(CRGB *leds, uint8_t batteryLevel, uint8_t progress) {
        uint8_t simpleLevel = batteryLevel / 2;
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = (i < simpleLevel) ? CRGB::Red : CRGB::Black;
        if (batteryLevel % 2 && simpleLevel <= NUM_LEDS - 1) {
            leds[simpleLevel + 1] = (progress > 255 / 2) ? CRGB::Red : CRGB::Black;
        }
    }

    void chargingStartAnimation(CRGB *leds, uint8_t progress) {
        CRGB arr[6] = {};
        uint8_t ledsToLight = map(progress, 0, 255, 0, 7);
        for (int i = 0; i < ledsToLight; i++) arr[i] = CRGB::Red;
        mirror(leds, arr);
    }

}


struct LedState {
    /// Modes that are below 0 are kinda special mode
    /// User should not get into them with normal button click - it should be a very long press or something
    /// Then, you can go out of them by pressing forward until you get to 0 (first normal one) etc
    typedef enum {
        Battery = -1,
        TransFlag,
        GenderqueerFlag,
        Rainbow,
        Pink,
        Torch,
        RedBlink,
        Black,
    } LedMode;
    /// Normal, non-special modes
    static const uint8_t AVAILABLE_MODES = 7;

    static unsigned long getTimeoutSeconds(LedMode mode, bool extended = false) {
        if (mode == LedMode::Torch) return extended ? (20 * 60) : (10 * 60);
        if (mode == LedMode::Black) return 20;
        if (mode == LedMode::RedBlink) return extended ? (90 * 60) : (45 * 60);
        return extended ? (45 * 60) : (5 * 60);
    }
};


#endif //MKLOVE_LEDUTILS_H
