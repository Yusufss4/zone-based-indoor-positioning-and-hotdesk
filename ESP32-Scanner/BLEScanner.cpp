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

#define BLE_INTERVAL 100
#define BLE_WINDOW 40

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      extern uint8_t bufferIndex;
      extern BeaconData buffer[];
      if (bufferIndex >= BUFFER_SIZE - 2) {
        return;
      }
      if (advertisedDevice.haveName() || advertisedDevice.haveServiceUUID())
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
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(BLE_INTERVAL);
  pBLEScan->setWindow(BLE_WINDOW);  // less or equal setInterval value
}

void BLEScannerLoop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  delete pMAC_Address;
  pMAC_Address = NULL;

}
