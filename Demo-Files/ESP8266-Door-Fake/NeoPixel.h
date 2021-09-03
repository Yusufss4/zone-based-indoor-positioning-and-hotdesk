/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef NeoPixel_h
#define NeoPixel_h

#ifdef __AVR__
#include <avr/power.h>
#endif


#include <Adafruit_NeoPixel.h>
#include "Arduino.h"
extern Adafruit_NeoPixel strip;

class NeoPixel
{
  public:
    NeoPixel();
    void setupNeoPixel();
    void changeLedStatusCalculateCapacity(uint32_t innerColor, uint32_t outerColor, uint32_t warningColor, int numOfPeople, int capacity);
    void changeLedStatusOneLedPerPerson(uint32_t innerColor, uint32_t outerColor, uint32_t warningColor, int numOfPeople, int capacity);
  private:
    int _pin;
};

#endif
