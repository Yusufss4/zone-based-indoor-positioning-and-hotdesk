#include <Arduino.h>
//This code is for Meeting or Kitchen ESP32s Scanning Gateway
//Topic list included in - https://docs.google.com/document/d/1ixmkAzqzE-u8imwFLdVT4XZ1hFTCaz8fN4PDlNe0_3w/edit?usp=sharing
//Device MAC: 84:0D:8E:2C:66:3C


//MQTT and WiFi
//#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define DATA_SEND 5000 //Per miliseconds
#define MQTT_MAX_PACKET_SIZE 1000

//Change Config File to Connect the MQTT Broker and WiFi
#include "MQTT_Config.h"

#define NUMBER_OF_STRING 4
#define MAX_STRING_SIZE 40

unsigned long last_time = 0;

//WiFiClientSecure wifiClient;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
//0000180a-0000-1000-8000-00805f9b34fb
//e54b0001-67f5-479e-8711-b3b99198ce6c
//A9A5941D-1681-14E8-E243-78685AB7D125
//E54B0001-67F5-479E-8711-B3B99198CE6C
//e0:5a:5a:c8:36:ac
static BLEUUID serviceUUID("0000180a-0000-1000-8000-00805f9b34fb");
static BLEUUID serviceUUID2("e54b0001-67f5-479e-8711-b3b99198ce6c");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("e54b0002-67f5-479e-8711-b3b99198ce6c");

static bool flag = true;
byte newValue = 0x00;

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      Serial.println("onDisconnect");
    }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID2);
  //BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");


  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  // Read the value of the characteristic.
  if (pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}
/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());

      // We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;

      } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks


void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("Connected to the WiFi.");
}


void callback(char* topic, byte* payload, unsigned int length) {


  //strcpy(DeviceMacAdress,WiFi.macAddress());


  Serial.print("Callback - ");
  Serial.print("Message:");
  //String messageTemp;
  char messageTemp[MAX_STRING_SIZE * NUMBER_OF_STRING];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    //messageTemp += (char)payload[i];
    //strcat(messageTemp,(char)payload[i]);
    messageTemp[i] = payload[i];
  }
  messageTemp[length] = '\0';
  Serial.print("\n");

  char *delimeter = ";";

  /* -- Getting MacAddresses from both Device and Message -- */
  char messageMacAddress[18];
  strcpy(messageMacAddress, strtok(messageTemp, delimeter));
  char deviceMacAddress[18];
  String getMAC = WiFi.macAddress();
  strcpy(deviceMacAddress, getMAC.c_str());

  Serial.print("deviceMacAddress: ");
  Serial.println(deviceMacAddress);


  char UUID[32];
  char eventStatus[10];
  char eventTime[6];
  int numOfPeopleInTheRoom = 0;
  int capacityOfRoom = 6;

  if (strcmp(messageMacAddress, deviceMacAddress) == 0) {
    // Check if the MQTT message was received on topic esp32/relay1
    if (strcmp(topic, "/nrom/yusuf") == 0) {
      Serial.println("Detected message at the topic nrom");
      numOfPeopleInTheRoom = atoi(strtok(NULL, delimeter));
      //changeLedStatus(numOfPeopleInTheRoom, capacityOfRoom);

      Serial.print("numOfPeopleInTheRoom: "); Serial.println(numOfPeopleInTheRoom);
    }
    if (strcmp(topic, "/next-event/yusuf") == 0) {
      Serial.println("Detected message at the topic next-event");
      strcpy(UUID, strtok(NULL, delimeter));
      strcpy(eventStatus, strtok(NULL, delimeter));
      strcpy(eventTime, strtok(NULL, delimeter));

      Serial.print("UUID: "); Serial.println(UUID);
      Serial.print("eventStatus: "); Serial.println(eventStatus);
      Serial.print("eventTime: "); Serial.println(eventTime);
    }
  }
  else {
    Serial.println("Different device");
  }

}

void setupMQTT() {
  //wifiClient.setCACert(ca_cert);
  mqttClient.setServer(MQTT_SERVER_NAME, MQTT_PORT);
  // set the callback function
  mqttClient.setCallback(callback);
  mqttClient.setKeepAlive(60);
}


void setup() {
  Serial.begin(115200);
  connectToWiFi();
  setupMQTT();
  //  log_d("Total heap: %d", ESP.getHeapSize());
  //  log_d("Free heap: %d", ESP.getFreeHeap());
  //  log_d("Total PSRAM: %d", ESP.getPsramSize());
  //  log_d("Free PSRAM: %d", ESP.getFreePsram());
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

}

void reconnectToTheBroker() {
  int numberOfConnectionsTried = 0;
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker..");
    if (mqttClient.connect(CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD)) {
      Serial.println("MQTT Broker Connected.");
      //subscribe to topic
      //mqttClient.subscribe("/nrom/yusuf");
      mqttClient.subscribe("/next-event/yusuf");
    }
    else {
      Serial.print("Connection failed, rc=");
      Serial.print(mqttClient.state());
      numberOfConnectionsTried++;
      if (numberOfConnectionsTried > 5) {
        Serial.print("Rebooting the device...");
        ESP.restart();
      }
    }
    delay(500);
  }
}


void loop() {


  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to the broker..");
    reconnectToTheBroker();
  }
  mqttClient.loop();

  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  else {
    Serial.println("doConnect = false");
  }



  unsigned long now = millis();
  if (now - last_time > DATA_SEND) {

    publishScanDataToMQTT();
    publishDeviceInfoToMQTT();

    if (connected) {
      String newString = "--ATA";
      //newValue = (newValue + 0x01) %3;
      Serial.println("Setting new characteristic value..");
      //Serial.println(newValue);
      Serial.println(newString);

      // Set the characteristic's value to be the array of bytes that is actually a string.
      pRemoteCharacteristic->writeValue(newString.c_str(), newString.length());

      //if(flag) {
      //pRemoteCharacteristic->writeValue(newValue);
      //flag = false
      //}
    } else if (doScan) {
      BLEDevice::getScan()->start(5,false);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }

    last_time = now;
  }

}


void publishScanDataToMQTT() {

  //  Serial.print("Publishing the data...");
  //  String payloadString = "{\"e\":[";
  //  for (uint8_t i = 0; i < numberOfDevicesFound; i++) {
  //    payloadString += "{\"m\":\"";
  //    payloadString += String(uniqueBuffer[i].address);
  //    payloadString += "\",\"r\":\"";
  //    payloadString += String(uniqueBuffer[i].rssi);
  //    payloadString += "\"}";
  //    if (i < bufferIndex - 1) {
  //      payloadString += ',';
  //    }
  //  }
  //  // SenML ends. Add this stations MAC
  //  payloadString += "],\"mac\":\"";
  //  payloadString += String(WiFi.macAddress()); payloadString += "\"}";
  //
  //  // Print and publish payload
  //  Serial.print("MAX len: ");
  //  Serial.println(MQTT_MAX_PACKET_SIZE);
  //
  //  Serial.print("Payload length: ");
  //  Serial.println(payloadString.length());
  //  Serial.println(payloadString);
  //
  //  uint8_t messageCharBuffer[MQTT_MAX_PACKET_SIZE];
  //  payloadString.getBytes(messageCharBuffer, payloadString.length() + 1);
  //
  //  payloadString.getBytes(message_char_buffer, payloadString.length() + 1);
  //  int result = mqttClient.publish("/o1/m1/esp32-1/info/yusuf", message_char_buffer, payloadString.length(), false);
  //  Serial.print("PUB Result: ");
  //  Serial.println(result);
  Serial.println(mqttClient.publish("/o1/m1/esp32-1/info/yusuf", "test"));
}

void publishDeviceInfoToMQTT() {
  //Not implemented. Temp and humidity.
  //result = client.publish("/o1/m1/esp32-1/info", message_char_buffer, payloadString.length(), false);
  //  Serial.print("PUB Result: ");
  //  Serial.println(result);
}
