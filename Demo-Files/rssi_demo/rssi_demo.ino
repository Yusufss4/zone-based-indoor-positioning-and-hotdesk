

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <BLEBeacon.h>
#include <cppQueue.h>
#define  IMPLEMENTATION  LIFO


typedef struct dev_list {
  String mac[5];
  cppQueue  rssi[5];
  
}dev_list_t;

BLEAddress add =BLEAddress("3c:71:bf:f5:5d:58");

cppQueue  q(sizeof(add), 50, IMPLEMENTATION); // Instantiate queue

static BLEAddress *ppMAC_Address;
int scanTime = 5; //In seconds
static BLEAddress *pMAC_Address;
BLEScan *pBLEScan;


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {

    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      if (advertisedDevice.haveServiceUUID() || advertisedDevice.haveName()) {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());
        q.push(advertisedDevice.getAddress());
      }

    }

};

void setup()
{
  //  BeaconData *head = (BeaconData* ) malloc(sizeof(BeaconData));
  Serial.begin(115200);
  //Serial.println("Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

}

void loop()
{
  //  print_list(head);
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  delete pMAC_Address;

  delay(500);
}
