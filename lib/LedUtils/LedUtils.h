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
    static CRGB LgbtFlag[NUM_LEDS] =
            {CRGB::Blue, CRGB::Green, CRGB::Yellow, Orange, CRGB::Red,
             Orange, CRGB::Red, Orange, CRGB::Yellow, CRGB::Green};
    // HSV -> 255/10=22,5 -> keep adding 22,5 until 255
    // Not used since we already have lgbt above that looks nice
    static CRGB Rainbow[NUM_LEDS] = {CHSV(0, 255, 255), CHSV(25, 255, 255), CHSV(51, 255, 255), CHSV(76, 255, 255),
                                     CHSV(102, 255, 255), CHSV(127, 255, 255), CHSV(153, 255, 255), CHSV(178, 255, 255),
                                     CHSV(204, 255, 255), CHSV(229, 255, 255)};
}


struct LedState {
    typedef enum {
        TransFlag = 0,
        GenderqueerFlag = 1,
        LgbtFlag = 2,
        Rainbow = 3,
        Red = 4,
        Torch = 5,
        Black = 6,
    } LedMode;
    static const uint8_t AVAILABLE_MODES = 7;

    static unsigned long getTimeoutSeconds(LedMode mode, bool extended = false) {
        if (mode == LedMode::Torch) return extended ? (10 * 60) : (5 * 60);
        if (mode == LedMode::Black) return 20;
        return extended ? (30 * 60) : (3 * 60);
    }
};


#endif //MKLOVE_LEDUTILS_H
