/*
  Morse.cpp - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/

#include "Arduino.h"
#include "NeoPixel.h"
NeoPixel::NeoPixel()
{
}

void NeoPixel::setupNeoPixel() {
  strip.begin();
  strip.setBrightness(50);
  strip.show();
}

void NeoPixel::changeLedStatusCalculateCapacity(uint32_t innerColor, uint32_t outerColor, uint32_t warningColor, int numOfPeople, int capacity)
{
  Serial.print("Multi File is working");

  //Calculate the capacity and increase the number of led per person.

  int ledPerPerson = (strip.numPixels() / capacity);
  Serial.print("\nLed Per People : ");
  Serial.println(ledPerPerson);

  if (numOfPeople >= capacity) {
    Serial.println("Room is full");
    strip.fill(warningColor);
    strip.show();
    return;
  }

  uint8_t wait = 0;
  if (numOfPeople == 0) {
    strip.fill(outerColor);
    strip.show();
  }
  else {
    int ledNumber = 0;
    strip.clear();  //will change later.
    strip.fill(outerColor, ledNumber, 0);
    for (int i = 0; i < numOfPeople; i++) {
      for (int j = 0; j < ledPerPerson; j++) {
        strip.setPixelColor(ledNumber, innerColor);
        ledNumber++;
      }

    }
    strip.show();
  }

}

void NeoPixel::changeLedStatusOneLedPerPerson(uint32_t innerColor, uint32_t outerColor, uint32_t warningColor, int numOfPeople, int capacity)
{
   Serial.print("Multi File is working");
  //1 led per person, if the capacity is reached all warningColor.

  if (numOfPeople >= capacity) {
    Serial.println("Room is full");
    strip.fill(warningColor);
    strip.show();
    return;
  }
  if (numOfPeople == 0) {
    strip.fill(outerColor);
    strip.show();
  }
  else {
    strip.fill(outerColor, numOfPeople - 1, strip.numPixels());
    for (uint16_t i = 0; i < numOfPeople; i++) {
      strip.setPixelColor(i, innerColor);
    }
    strip.show();
  }
}
