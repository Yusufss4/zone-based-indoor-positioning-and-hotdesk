#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

//What if pixel count is lower than the capacity?

#define NEOPIXEL_PIN 13
#define PIXEL_COUNT 24

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {

  uint32_t orange = strip.Color(255, 69, 0);
  uint32_t black = strip.Color(0, 0, 0);
  uint32_t blue = strip.Color(0, 0, 255);
  uint32_t red = strip.Color(255, 0, 0);
  uint32_t capacityOfRoom = 6;

  Serial.println("\n1st Loop");
  capacityOfRoom = 12;
  for (uint16_t numOfPeople = 0; numOfPeople <= 12; numOfPeople++) {
    changeLedStatus(orange, black, red, numOfPeople, capacityOfRoom);
  }

  Serial.println("\n2nd Loop");
  capacityOfRoom = 12;
  for (uint16_t numOfPeople = 12; numOfPeople > 0; numOfPeople--) {
    changeLedStatus(orange, black, red, numOfPeople, capacityOfRoom);
  }

  Serial.println("\n3th Loop");
  capacityOfRoom = 6;
  for (uint16_t numOfPeople = 0; numOfPeople <= 12; numOfPeople++) {
    changeLedStatus(orange, black, red, numOfPeople, capacityOfRoom);
  }

  Serial.println("\n4th Loop");
  capacityOfRoom = 6;
  for (uint16_t numOfPeople = 12; numOfPeople > 0; numOfPeople--) {
    changeLedStatus(orange, black, red, numOfPeople, capacityOfRoom);
  }

  Serial.println("\n5th Loop");
  capacityOfRoom = 3;
  for (uint16_t numOfPeople = 0; numOfPeople <= 12; numOfPeople++) {
    changeLedStatus(orange, black, red, numOfPeople, capacityOfRoom);
  }

  Serial.println("\n6th Loop");
  capacityOfRoom = 3;
  for (uint16_t numOfPeople = 12; numOfPeople > 0; numOfPeople--) {
    changeLedStatus(orange, black, red, numOfPeople, capacityOfRoom);
  }

}

void colorWipeV2(uint32_t c, uint8_t wait, uint8_t numOfPeople) {
  uint32_t black = strip.Color(0, 0, 0);
  if (numOfPeople == 0) {
    uint32_t black = strip.Color(0, 0, 0);
    strip.fill(black);
    strip.show();
    delay(wait);
  }
  else {
    strip.fill(black, numOfPeople - 1, strip.numPixels());
    for (uint16_t i = 0; i < numOfPeople; i++) {
      strip.setPixelColor(i, c);
    }
    strip.show();
    delay(wait);
  }
}

void changeLedStatus(uint32_t innerColor, uint32_t outerColor, uint32_t warningColor, int numOfPeople, int capacity) {

  uint32_t black = strip.Color(0, 0, 0);
  int ledPerPerson = (strip.numPixels() / capacity);
  Serial.print("\nLed Per People : ");
  Serial.println(ledPerPerson);

  if (numOfPeople >= capacity) {
    Serial.println("Room is full");
    strip.fill(warningColor);
    strip.show();
    return;
  }

  uint8_t wait = 500;
  if (numOfPeople == 0) {
    strip.fill(outerColor);
    strip.show();
    delay(wait);
  }
  else {
    delay(wait);
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
    delay(wait);
  }

}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
