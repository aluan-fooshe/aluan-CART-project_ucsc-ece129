/*
 *  Morse.h - Library for flashing Morse code
    Created by Audrey on 4/27/2026
    (basic example)
*/

#ifndef MORSE_H
#define MORSE_H

#include "Arduino.h"

class Morse
{
public:
    Morse(int pin);
    void begin();
    void dot();
    void dash();
private:
    int _pin;
};

#endif //MORSE_H  ✅