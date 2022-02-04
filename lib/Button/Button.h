#ifndef MKLOVE_BUTTON_H
#define MKLOVE_BUTTON_H

class Button {
public:
    typedef void (*callback_t)();

    Button(uint8_t pin) {
        _pin = pin;
    }

    ~Button() = default;

    void begin() {
        pinMode(_pin, INPUT);
        _lastState = !digitalRead(_pin);
        _lastStateChange = millis();
    }

    void onPressed(callback_t callback) {
        _pressedCallback = callback;
    }

    void onPressedFor(uint32_t duration, callback_t callback) {
        _pressedForCallback = callback;
        _onPressedForTime = duration;
    }

    bool pressed() {
        return !digitalRead(_pin);
    }

    bool pressedFor(uint32_t duration) {
        return !digitalRead(_pin) && (millis() - _lastStateChange) > duration;
    }

    void read() {
        bool state = !digitalRead(_pin);
        if (state != _lastState) {
            _lastStateChange = millis();
        }
        if (state) {
            if (_pressedForCallback && !_onPressedForRan && _pressedForTimeElapsed()) {
                _pressedForCallback();
                _onPressedForRan = true;
            }
        } else {
            // If current state is "released" and last one was "pressed"
            if (_lastState && !_onPressedForRan) {
                _pressedCallback();
            }
            _onPressedForRan = false;
        }

        _lastState = state;
    }


private:
    uint8_t _pin;
    bool _lastState = false;
    uint32_t _lastStateChange = 0;
    callback_t _pressedCallback = nullptr;
    callback_t _pressedForCallback = nullptr;
    uint32_t _onPressedForTime = 1000;
    bool _onPressedForRan = false;

    bool _pressedForTimeElapsed() {
        return millis() - _lastStateChange > _onPressedForTime;
    }
};

#endif //MKLOVE_BUTTON_H
