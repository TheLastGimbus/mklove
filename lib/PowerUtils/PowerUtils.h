//
// Created by matiii on 02.02.2022.
//

#ifndef MKLOVE_POWERUTILS_H
#define MKLOVE_POWERUTILS_H


void reboot() {
    noInterrupts(); // disable interrupts which could mess with changing prescaler
    CLKPR = 0b10000000; // enable prescaler speed change
    CLKPR = 0; // set prescaler to default (16mhz) mode required by bootloader
    void (*ptrToFunction)(); // allocate a function pointer
    ptrToFunction = 0x0000; // set function pointer to bootloader reset vector
    (*ptrToFunction)(); // jump to reset, which bounces in to bootloader
}

void disableAdc() { ADCSRA &= ~_BV(ADEN); }

void enableAdc() { ADCSRA |= _BV(ADEN); }

int getVcc() {
    //reads internal 1V1 reference against VCC
#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44__)
    ADMUX = _BV(MUX5) | _BV(MUX0); // For ATtiny84
#elif defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__)
    ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85/45
#elif defined(__AVR_ATmega1284P__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega1284
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
#endif
    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Convert
    while (bit_is_set(ADCSRA, ADSC));
    uint8_t low = ADCL;
    unsigned int val = (ADCH << 8) | low;
    //discard previous result
    ADCSRA |= _BV(ADSC); // Convert
    while (bit_is_set(ADCSRA, ADSC));
    low = ADCL;
    val = (ADCH << 8) | low;

    return ((long) 1024 * 1100) / val;
}

#endif //MKLOVE_POWERUTILS_H
