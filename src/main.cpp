#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "FastLED.h"
#include <LedUtils.h>
#include <EasyButton.h>
#include <PowerUtils.h>

#define PIN_BTN 2
#define PIN_LEDS 1
#define PIN_MOSFET 4
#define PIN_USB_PLUS 3

EasyButton btn(PIN_BTN, 150, false, true);

#define NUM_LEDS 10
CRGB leds[NUM_LEDS];

// State management
int8_t currentState = 0;
/// Lower brightness, but longer timeout
bool extendedMode = true;  // This is because main use-case will probably at night
/// From 0 to 10
uint8_t batteryLevel = 5;
bool isCharging = false;

unsigned long lastInteraction = 0;

void interaction() { lastInteraction = millis(); }

/// Updates battery level
/// Returns true if was plugged in to charger (one time)
void updateBattery() { batteryLevel = map(getVcc(), 3000, 4200, 0, NUM_LEDS - 1); }

void playChargingAnimationBlocking() {
    for (uint8_t i = 0; i < 255; i++) {
        ledutils::chargingStartAnimation(leds, i);
        FastLED.show();
        delay(7);
    }
}

void wake() {
    detachInterrupt(0);
    sleep_disable();
    interaction();
}

void _preSleep() {
    disableAdc();
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(PIN_LEDS, INPUT_PULLUP);
    digitalWrite(PIN_MOSFET, LOW);
}

/// Go to deep sleep with interrupt wakeup
void sleepNow() {
    _preSleep();

    attachInterrupt(0, wake, LOW);
    noInterrupts();  // This should (?) make it more reliable ??
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    interrupts();
    sleep_cpu();
    sleep_disable();
    detachInterrupt(0);

    interaction();
}

void onPress() {
    // If last press was >15 seconds ago - user probably wants to turn off instead of switching modes again
    if (millis() - lastInteraction > 15 * 1000) {
        currentState = LedState::Black;
    } else {
        currentState++;
        if (currentState >= LedState::AVAILABLE_MODES) currentState = 0;
    }
    interaction();
}

void onLongPress() {
    interaction();
    extendedMode = !extendedMode;
}


void setup() {
    detachInterrupt(0);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_LEDS, OUTPUT);
    pinMode(PIN_MOSFET, OUTPUT);
    pinMode(PIN_USB_PLUS, INPUT);
    btn.begin();
    disableAdc();

    digitalWrite(LED_BUILTIN, HIGH);            // sets the LED on
    digitalWrite(PIN_MOSFET, HIGH);            // gives LEDs power

    FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NUM_LEDS);

    btn.onPressed(onPress);
    btn.onPressedFor(500, onLongPress);
    interaction();
}

void loop() {
    // This is *the big switch* that takes care of setting proper led colors according to "global state variables"
    // (currentState, extendedMode, batteryLevel, isCharging) etc.
    uint8_t brightness = 50;
    switch (currentState) {
        case LedState::TransFlag:
            ledutils::transFlag(leds);
            break;
        case LedState::Green:
            fill_solid(leds, NUM_LEDS, CRGB::Green);
            break;
        case LedState::Blue:
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
            break;
        case LedState::Rainbow:
            // millis() / X <- X dictates speed - lower is faster
            ledutils::rainbow(leds, NUM_LEDS, (millis() / 18) % 255);
            break;
        case LedState::Pink:
            fill_solid(leds, NUM_LEDS, ledutils::Pink);
            brightness = 30;
            break;
        case LedState::Torch:
            fill_solid(leds, NUM_LEDS, CRGB::White);
            brightness = 255;
            break;
        case LedState::RedBlink:
            // same case as for rainbow
            ledutils::redBlink(leds, (millis() / 4) % 255);
            brightness = 255;
            break;
        case LedState::Battery:
            ledutils::batteryIndicator(leds, batteryLevel);
            break;
        case LedState::Black:
        default:
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            break;
    }
    FastLED.setBrightness(extendedMode ? brightness / 10 : brightness);
    FastLED.show();

    btn.read();

    // If pressed *very* long, go to "special" modes
    // Note: manual because multiple onPressedFor don't work :/
    if (btn.pressedFor(1500)) currentState = LedState::Battery;

    // Note: exporting this to separate function takes 12 bytes (I know :O) so imma leave it here
    // This is (I think) 100% reliable of detecting charging, instead of previous "check voltage jumps" method
    // It is possible thanks to re-soldering R3 pull-up resistor directly to 5V form usb instead of global VCC
    bool _newCharging = digitalRead(PIN_USB_PLUS);
    // Is charging but wasn't last time => started charging
    if (_newCharging && !isCharging) {
        currentState = LedState::Battery;
        playChargingAnimationBlocking();
    }
    isCharging = _newCharging;

    // Comment: DAMN those "EVERY_N_" macros use a lot of flash :eyes:
    // - but, my own implementation used pretty much the same

    // If state==battery, update lvl every loop (sometimes gives nice "flickering" effect) - else, only every 10s
    if (currentState == LedState::Battery) {
        enableAdc();
        updateBattery();
    } else {
        EVERY_N_SECONDS(10) {
            enableAdc();
            // There was an idea to also play animation + set currentState=Battery
            // But turned out that power jumps while switching from "Torch" to anything else can also trigger this :/
            updateBattery();
            disableAdc();
        }
    }
    // If battery is low, show it to the user every 3 minutes
    EVERY_N_MINUTES(3) {
        if (batteryLevel <= 2) currentState = LedState::Battery;
    }

    // THIS IS *THE* THING that takes care of going to sleep - it's very important!
    if (millis() - lastInteraction >
        LedState::getTimeoutSeconds(static_cast<LedState::LedMode>(currentState), extendedMode) * 1000) {
        sleepNow();
    }
}

