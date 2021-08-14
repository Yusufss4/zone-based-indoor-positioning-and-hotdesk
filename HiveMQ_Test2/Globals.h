#ifndef Globals_h
#define Globals_h
#include <Arduino.h>

typedef struct {
  char address[17];   // 67:f1:d2:04:cd:5d
  int rssi;
} BeaconData;

extern uint8_t bufferIndex;  // Found devices counter
extern BeaconData buffer[]; 

#endif
