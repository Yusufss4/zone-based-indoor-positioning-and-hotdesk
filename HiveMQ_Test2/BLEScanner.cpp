//
// BLEScanner.c
//

#include "BLEScanner.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>

int scanTime = 10; //In seconds
static BLEAddress *pMAC_Address;
BLEScan* pBLEScan;


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      if (advertisedDevice.haveName())
      {
        Serial.print("MAC Adress: ");
        pMAC_Address = new BLEAddress(advertisedDevice.getAddress());
        Serial.println(pMAC_Address->toString().c_str());
        Serial.print("RSSI: ");
        Serial.println(advertisedDevice.getRSSI());
        Serial.println("");
      }
     }
};

// This initialises the six servos
void BLEScannerSetup() {
  Serial.println("Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

// This is how to use the six servos to form the passed-in pattern
void BLEScannerLoop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  delete pMAC_Address;
  pMAC_Address = NULL;
} 
