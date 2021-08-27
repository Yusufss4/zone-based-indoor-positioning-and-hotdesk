

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <BLEBeacon.h>
#include <stdlib.h>
#include<stdio.h>
#define DATA_SIZE 5
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))
#define DEVICE_NUMBER 5
static BLEAddress *ppMAC_Address;
int scanTime = 5; //In seconds
static BLEAddress *pMAC_Address;
BLEScan *pBLEScan;

typedef struct n {
  char MAC[17];
  int rssi[DATA_SIZE] = {0};
  struct n *next;
} BeaconData;

//struct n;
BeaconData *iter = NULL;
BeaconData *head = NULL;
int dataCount = 0;




BeaconData * find(BeaconData * head, const char MAC[]) {
  BeaconData * current = head;
  //         ppMAC_Address = new BLEAddress(advertisedDevice.getAddress());
  while (current->next != NULL) {
    if (strcmp(MAC, current->MAC) == 0) {
      break;
    }
    current = current->next;
  }
  return current;
}

void print_list(BeaconData * head) {
  BeaconData * current = head;
  int i = 0;
  while (current->next != NULL) {
    printf("%s\n", current->MAC);
    for (i = 0; i < DATA_SIZE; i++) {
      printf("%d\n", current->rssi[i]);

    }
    current = current->next;
  }
}

void push(BeaconData * head, const char MAC[]) {
  BeaconData * current = head;

  while (current->next != NULL) {
    current = current->next;
  }
  current->next = (BeaconData *) malloc(sizeof(BeaconData));
  strcpy(current->next-> MAC, MAC);
  //        current->next-> MAC = MAC;
  current->next->next = NULL;
}



class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{


    //  void print_list(BeaconData * head) {
    //    BeaconData * current = head;
    //    int i = 0;
    //    while (current->next != NULL) {
    //        printf("%s\n",current->MAC);
    //        printf("%d\n", current->rssi[i]);
    //        i++;
    //        current = current->next;
    //    }
    //}
    //
    //  void push(BeaconData * head, char MAC[]) {
    //        BeaconData * current = head;
    //
    //    while (current->next != NULL) {
    //        current = current->next;
    //    }
    //        current->next = (BeaconData *) malloc(sizeof(BeaconData));
    //        strcpy(current->next-> MAC, MAC);
    ////        current->next-> MAC = MAC;
    //        current->next->next = NULL;
    //    }

    //  BeaconData * find(BeaconData * head, char MAC[]) {
    //         BeaconData * current = head;
    //         ppMAC_Address = new BLEAddress(advertisedDevice.getAddress());
    //       while (current->next != NULL) {
    //           if(strcmp(ppMAC_Address->toString().c_str(), current->MAC) == 0){
    //                break;
    //           }
    //           current = current->next;
    //    }
    //           return current;
    //   }
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {

      if (advertisedDevice.haveServiceUUID() || advertisedDevice.haveName())
      {

        BeaconData *head = (BeaconData* ) malloc(sizeof(BeaconData));
        if (head == NULL) {
          printf("Can't allocate head\n");
          return ;
        }
        head->next = NULL;
        pMAC_Address = new BLEAddress(advertisedDevice.getAddress());
        strcpy(head->MAC, pMAC_Address->toString().c_str());


        iter = find(head, pMAC_Address->toString().c_str());
        if (iter->next == NULL ) {
          push(head, pMAC_Address->toString().c_str());
        }
        else {
          for (int j = 0; j < DATA_SIZE; j++) {
            iter->rssi[j] = advertisedDevice.getRSSI();


            //                      if(rssi[j] == 0){
            //                         break;
            //                      }
            //                   dataCount++;

          }

        }

      }




      //          iter->next = (BeaconData*)malloc(sizeof(BeaconData));
      //          iter = iter->next;
      //          pMAC_Address = new BLEAddress(advertisedDevice.getAddress());
      //          strcpy(iter->MAC, pMAC_Address->toString().c_str()); //iter->MAC = advertisedDevice.getAddress();
      //          iter->rssi = advertisedDevice.getRSSI();
      ////          iter->rssi = 55;
      //          iter->next = NULL;
      //               }
      //           print_list(head);
      //      int i = 0;
      //      iter = head;
      //      while (iter->next != NULL)
      //      {
      //
      //        Serial.println("MAC Adress:");
      //        Serial.println(iter->MAC);
      //        Serial.println("RSSI:");
      //        Serial.println( iter->rssi[0]);
      //        i++;
      //        iter = iter->next;
      //      }
      //
      //        pMAC_Address = new BLEAddress(advertisedDevice.getAddress());
      //        Serial.println(pMAC_Address->toString().c_str());
      //        Serial.print("RSSI: ");
      //        Serial.println(iter->rssi);
      //        Serial.println("");
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
  free(head);
  pMAC_Address = NULL;
  delay(500);
}
