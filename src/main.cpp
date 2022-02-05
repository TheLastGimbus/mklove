#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "FastLED.h"
#include <LedUtils.h>
#include <EasyButton.h>
#include <PowerUtils.h>

#define PIN_BTN 2
#define PIN_LEDS 0
#define PIN_MOSFET 1

EasyButton btn(PIN_BTN, 150, false, true);

#define NUM_LEDS 10
CRGB leds[NUM_LEDS];

// State management
int8_t currentState = 0;
/// Lower brightness, but longer timeout
bool extendedMode = true;  // This is because main use-case will probably at night
/// From 0 to 10
uint8_t batteryLevel = 5;
long lastBattery = 5000;
#define BATTERY_INCREASE_CHARGING_THRESHOLD 80  // This may be very subjective of battery capacity and level :/

unsigned long lastInteraction = 0;

void interaction() { lastInteraction = millis(); }

/// Updates battery level
/// Returns true if was plugged in to charger (one time)
bool updateBattery() {
    long battery = getVcc();
    batteryLevel = map(battery, 3000, 4200, 0, NUM_LEDS - 1);
    if ((battery - lastBattery) > BATTERY_INCREASE_CHARGING_THRESHOLD) {
        lastBattery = battery;
        return true;
    } else {
        lastBattery = battery;
        return false;
    }
}

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

void onLongPress() {
    interaction();
    extendedMode = !extendedMode;
}


void setup() {
    detachInterrupt(0);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_LEDS, OUTPUT);
    pinMode(PIN_MOSFET, OUTPUT);
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
    uint8_t brightness = 30;
    switch (currentState) {
        case LedState::TransFlag:
            ledutils::transFlag(leds);
            break;
        case LedState::Green:
            fill_solid(leds, NUM_LEDS, CRGB::Green);
            break;
        case LedState::GenderqueerFlag:
            ledutils::genderqueerFlag(leds);
            break;
        case LedState::Rainbow:
            // millis() / X <- X dictates speed - lower is faster
            ledutils::rainbow(leds, NUM_LEDS, (millis() / 18) % 255);
            break;
        case LedState::Pink:
            fill_solid(leds, NUM_LEDS, ledutils::Pink);
            break;
        case LedState::Torch:
            fill_solid(leds, NUM_LEDS, CRGB::White);
            brightness = 255;
            break;
        case LedState::RedBlink:
            // same case as for rainbow
            ledutils::redBlink(leds, (millis() / 5) % 255);
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
    // because multiple onPressedFor don't work :/
    if (btn.pressedFor(1500)) {
        enableAdc();
        // Show the indicator and update battery - may prevent false-positive trigger (of charging animation)
        // because of jump in power usage
        ledutils::batteryIndicator(leds, batteryLevel);
        FastLED.show();
        delay(10);
        updateBattery();
        currentState = LedState::Battery;
    }

    // If state==battery, update lvl every loop (sometimes gives nice "flickering" effect) - else, only every 10s
    if (currentState == LedState::Battery) {
        if (updateBattery()) playChargingAnimationBlocking();
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

    if ((millis() - lastInteraction >
         LedState::getTimeoutSeconds(static_cast<LedState::LedMode>(currentState), extendedMode) * 1000)) {
        sleepNow();
    }
}

