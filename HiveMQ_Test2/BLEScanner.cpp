//
// BLEScanner.c
//

#include "BLEScanner.h"
#include "Globals.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
//#include <BLEBeacon.h>

int scanTime = 10; //In seconds
static BLEAddress *pMAC_Address;
BLEScan* pBLEScan;


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
    extern uint8_t bufferIndex;
    extern BeaconData buffer[];
    if(bufferIndex >= 48) {
      return;
    }
      if (advertisedDevice.haveName())
      {
        pMAC_Address = new BLEAddress(advertisedDevice.getAddress());
        buffer[bufferIndex].rssi = advertisedDevice.getRSSI();
        strcpy (buffer[bufferIndex].address, advertisedDevice.getAddress().toString().c_str());
        bufferIndex++;
      }
     }
};

void BLEScannerSetup() {
  Serial.println("Scanning...");
}

void BLEScannerLoop() {
  
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(40);  // less or equal setInterval value
  
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  delete pMAC_Address;
  pMAC_Address = NULL;
  
} 
