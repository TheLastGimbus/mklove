#ifndef MKLOVE_LEDUTILS_H
#define MKLOVE_LEDUTILS_H

#include <FastLED.h>

#define NUM_LEDS 10

CRGB TransBlue = 0x000aff;
CRGB TransPink = 0xff0a82;

namespace ledutils {
    typedef enum {
        TransBlue = 0x000aff,
        TransPink = 0xff0a82,
        Orange = 0xff3300,
        Pink = 0xff1010,
        Purple = 0x6600ff,
    } Colors;
    // BLUE, PINK, WHITE, PINK, BLUE
    static CRGB TransFlag[NUM_LEDS] =
            {TransBlue, TransPink, CRGB::White, TransPink, TransBlue,
             TransPink, TransBlue, TransPink, CRGB::White, TransPink};
    // YELLOW, WHITE, VIOLET, BLACK
    // Ugly as fuck, do not use
    static CRGB NonbinaryFlag[NUM_LEDS] =
            {CRGB::DarkGray, CRGB::Black, CRGB::Purple, CRGB::White, CRGB::Yellow,
             CRGB::White, CRGB::Yellow, CRGB::White, CRGB::Purple, CRGB::Black};

    // PURPLE, WHITE, GREEN
    static CRGB GenderqueerFlag[NUM_LEDS] =
            {CRGB::Green, CRGB::Green, CRGB::White, Purple, Purple,
             Purple, Purple, Purple, CRGB::White, CRGB::Green};

    // RED, ORANGE, YELLOW, GREEN, BLUE
    // Not used since we already have rainbow below
    static CRGB LgbtFlag[NUM_LEDS] =
            {CRGB::Blue, CRGB::Green, CRGB::Yellow, Orange, CRGB::Red,
             Orange, CRGB::Red, Orange, CRGB::Yellow, CRGB::Green};

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

}


struct LedState {
    typedef enum {
        TransFlag,
        GenderqueerFlag,
        Rainbow,
        Pink,
        Torch,
        RedBlink,
        Black,
    } LedMode;
    static const uint8_t AVAILABLE_MODES = 7;

    static unsigned long getTimeoutSeconds(LedMode mode, bool extended = false) {
        if (mode == LedMode::Torch) return extended ? (20 * 60) : (10 * 60);
        if (mode == LedMode::Black) return 20;
        if (mode == LedMode::RedBlink) return extended ? (90 * 60) : (45 * 60);
        return extended ? (45 * 60) : (5 * 60);
    }
};


#endif //MKLOVE_LEDUTILS_H
