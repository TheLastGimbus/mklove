/// This file contains utilities for nice LED animations, as well as their state-management

#ifndef MKLOVE_LEDUTILS_H
#define MKLOVE_LEDUTILS_H

#include <FastLED.h>

#define NUM_LEDS 10

/// This provides utilities for nice animations etc - all of them are standalone, don't do FastLED.show() by themselves
/// They are dependent on NUM_LEDS tho
///
/// Any animations - instead of delay() - use `uint_8 progress` value
/// - you can pass something like `(millis() / 10) % 255` in there
namespace ledutils {
    /// My own colors
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
    void redBlink(CRGB *leds, uint8_t progress) {
        fill_solid(leds, NUM_LEDS, (progress < 35 || (progress > 70 && progress < 105)) ? CRGB::Red : CRGB::Black);
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


/// Here is some state-management
struct LedState {
    /// Modes that are below 0 are a-bit-special modes
    /// User should not get into them with normal button click - it should be a very long press or something
    /// Then, you can go out of them by pressing forward until you get to 0 (first normal one) etc
    ///
    /// Reason for all of this, is that the heart should be quick to use - as a night-light or torch
    /// - you don't want to scroll thorough lgbt flags when you just want quick light
    typedef enum {
        Battery = -6,
        Blue = -5,
        Green = -4,
        TransFlag = -3,
        Rainbow = -2,
        RedBlink = -1,
        Pink,
        Torch,
        Black,
    } LedMode;
    /// Normal, non-special modes
    static const uint8_t AVAILABLE_MODES = 3;

    static unsigned long getTimeoutSeconds(LedMode mode, bool extended = false) {
        if (mode == LedMode::Torch) return extended ? (30 * 60) : (15 * 60);
        if (mode == LedMode::Black) return 60;
        if (mode == LedMode::RedBlink) return (60 * 60);
        return extended ? (30 * 60) : (45 * 60);
    }
};


#endif //MKLOVE_LEDUTILS_H
