# mklove :sparkling_heart:
...not war :broken_heart:

Here lays the code for ATiny85 that powers `mklove` - cute little LED heart

It is based on Digistump usb-capable board - not just pure ATtiny

Many parts of the code are heavily reliant on many hardware modifications that I've done on Digistump pcb board. Some examples:
- detecting whether device is currently charging is possible thanks to re-soldering the R3 pull-up resistor
- ...and this was possible by de-soldering D3 diode that was connecting USB with VCC

If you would ever want to re-create it yourself, or just read into the story of creating it, you can do it on my blog ^_^

https://the.lastgimbus.com/blog/crafting-battery-powered-thingy-with-attiny85

## Code details

Thanks to my expertise with Flutter :relieved::relieved::relieved: I've learned a lot about state management - and thus, the main `void loop() {}` is running always-and-smooth - without excessive use of `delay()`

// ONLY ONE part that uses blocking `delay()` is "charging started" animation, because I didn't have idea nor flash memory (xD) to do this otherwise

Everything else is done with global variables that hold state. You modify those variables, and then `loop()` will take care of rest.

Time-based things are done with `progress` concept. If you want a blink, instead of doing:
```cpp
void loop(){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
```

You do:
```cpp
void blink(uint8_t progress){
    if (progress > (255/2)) digitalWrite(LED_BUILTIN, HIGH);
    else digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    blink(millis() % 255);
    // You could also divide or multiply millis() to slow down or speed up the animation:
    blink((millis() / 10) % 255);
}
```

The `EasyButton` library really helped with this! https://github.com/evert-arias/EasyButton
