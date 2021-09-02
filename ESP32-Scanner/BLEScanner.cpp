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

int scanTime = 1; //In seconds
static BLEAddress *pMAC_Address;
BLEScan* pBLEScan;

#define BLE_INTERVAL 100
#define BLE_WINDOW 99

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
        //Serial.println("strcpy (buffer[bufferIndex].address, advertisedDevice.getAddress().toString().c_str());");
        strcpy (buffer[bufferIndex].address, advertisedDevice.getAddress().toString().c_str());
        bufferIndex++;
      }
    }
};

void BLEScannerSetup() {
  Serial.println("Scanning...");
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(),true);
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(BLE_INTERVAL);
  pBLEScan->setWindow(BLE_WINDOW);  // less or equal setInterval value
}

void BLEScannerLoop() {
  Serial.println("BLEScanResults foundDevices = pBLEScan->start(scanTime, false)");
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println("pBLEScan->clearResults();");
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  Serial.println("delete pMAC_Address;");
}
