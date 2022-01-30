#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "FastLED.h"
#include <LedUtils.h>
#include <EasyButton.h>

#define DEBUG 1

#define PIN_BTN 2
#define PIN_LEDS 0
#define PIN_MOSFET 1

EasyButton btn(PIN_BTN, 35, false, true);

#define NUM_LEDS 10
CRGB leds[NUM_LEDS];

// State management
uint8_t currentState = 0;
bool lowPowerLongTime = false;

unsigned long lastInteraction = 0;

void interaction() { lastInteraction = millis(); }

void reboot() {
    noInterrupts(); // disable interrupts which could mess with changing prescaler
    CLKPR = 0b10000000; // enable prescaler speed change
    CLKPR = 0; // set prescaler to default (16mhz) mode required by bootloader
    void (*ptrToFunction)(); // allocate a function pointer
    ptrToFunction = 0x0000; // set function pointer to bootloader reset vector
    (*ptrToFunction)(); // jump to reset, which bounces in to bootloader
}

void wake() {
    detachInterrupt(0);
    sleep_disable();
    interaction();
}

void _preSleep() {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(PIN_LEDS, HIGH);
    digitalWrite(PIN_MOSFET, LOW);
}

/// Go to deep sleep with interrupt wakeup
void sleepNow() {
    _preSleep();

    attachInterrupt(0, wake, LOW);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
    sleep_disable();
    detachInterrupt(0);

    interaction();
}

void onPress() {
    interaction();
    currentState++;
    if (currentState >= LedState::AVAILABLE_MODES) currentState = 0;
}

void onDoublePress() {
    interaction();
#if DEBUG
    reboot();
#endif
}

void onLongPress() {
    interaction();
    lowPowerLongTime = !lowPowerLongTime;
}


void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_LEDS, OUTPUT);
    pinMode(PIN_MOSFET, OUTPUT);
    btn.begin();
    detachInterrupt(0);

    digitalWrite(LED_BUILTIN, HIGH);            // sets the LED on
    digitalWrite(PIN_MOSFET, HIGH);            // gives LEDs power

    FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NUM_LEDS);

    btn.onPressed(onPress);
    btn.onSequence(2, 200, onDoublePress);
    btn.onPressedFor(1500, onLongPress);
    interaction();
}

void setLedsFromArray(CRGB array[NUM_LEDS]) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = array[i];
    }
}

void loop() {
    uint8_t brightness = 30;
    switch (currentState) {
        case LedState::TransFlag:
            setLedsFromArray(ledutils::TransFlag);
            break;
        case LedState::GenderqueerFlag:
            setLedsFromArray(ledutils::GenderqueerFlag);
            break;
        case LedState::Rainbow:
            // millis() / X <- X dictates speed
            ledutils::rainbow(leds, NUM_LEDS, (millis() / 18) % 255);
            break;
        case LedState::Red:
            for (auto &led: leds)led = CRGB::Red;
            break;
        case LedState::Torch:
            for (auto &led: leds)led = CRGB::White;
            brightness = 255;
            break;
        case LedState::Black:
        default:
            for (auto &led: leds)led = CRGB::Black;
            break;
    }
    FastLED.setBrightness(lowPowerLongTime ? brightness / 10 : brightness);
    FastLED.show();

    btn.read();

    unsigned long timeout =
            LedState::getTimeoutSeconds(static_cast<LedState::LedMode>(currentState), lowPowerLongTime) * 1000;

    if ((millis() - lastInteraction > timeout) && btn.releasedFor(5 * 1000)) {
        sleepNow();
    }
}

