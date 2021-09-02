#include <Arduino.h>
//This code is for Meeting or Kitchen ESP32s Scanning Gateway
//Topic list included in - https://docs.google.com/document/d/1ixmkAzqzE-u8imwFLdVT4XZ1hFTCaz8fN4PDlNe0_3w/edit?usp=sharing

//MQTT and WiFi
//#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

//Bluetooth Scan
#include "BLEScanner.h"
#include "Globals.h"

#define DATA_SEND 5000 //Per miliseconds
#define LOCAL_MQTT_MAX_PACKET_SIZE 2000
#define RESET_PIN 26 //17
#define WIFI_INFO_LED 27 //12 - BLUE First Led
#define MQTT_INFO_LED 14 //14 - GREEN Second Led
#define BLE_INFO_LED 12 //27 - YELLOW Third Led 

//Change Config File to Connect the MQTT Broker and WiFi
#include "MQTT_Config.h"

unsigned long last_time = 0;

//WiFiClientSecure wifiClient;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiManager wm;

uint8_t bufferIndex = 0; // Found devices counter
BeaconData buffer[BUFFER_SIZE];
uint8_t message_char_buffer[LOCAL_MQTT_MAX_PACKET_SIZE];
char deviceMacAddress[18];
char CLIENT_ID[18];
char APName[25];

void setAPandWiFiName() {
  String getMAC = WiFi.macAddress();
  strcpy(deviceMacAddress, getMAC.c_str());
  strcpy(CLIENT_ID, deviceMacAddress);
  String getAPName = "Fora - ";
  getAPName += WiFi.macAddress() ;
  strcpy(APName, getAPName.c_str());
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) {
    wm.setConnectTimeout(30);
    wm.setConfigPortalTimeout(120);
    bool res = wm.autoConnect(APName, "academy2021");
    if (!res) {

      Serial.println("Failed to connect");
      ESP.restart();
    }
    else {
      Serial.println("connected to wifi");
    }
  }
}
/*void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("Connected to the WiFi.");
  }*/

#ifndef ESP32_SCANNER
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print("\n");
}
#endif

void setupMQTT() {
  //wifiClient.setCACert(ca_cert);
  mqttClient.setServer(MQTT_SERVER_NAME, MQTT_PORT);
  /*if (mqttClient.setBufferSize(LOCAL_MQTT_MAX_PACKET_SIZE)) {
    Serial.print("Buffer size set to: "); Serial.println(mqttClient.getBufferSize());
    }
    else {
    Serial.print("Cant set the buffer..:(");
    }*/
  // set the callback function
#ifndef ESP32_SCANNER
  mqttClient.setCallback(callback);
#endif
  mqttClient.setKeepAlive(60);
}

void setupIO() {
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(WIFI_INFO_LED, OUTPUT);
  pinMode(MQTT_INFO_LED, OUTPUT);
  pinMode(BLE_INFO_LED, OUTPUT);
  digitalWrite(WIFI_INFO_LED, LOW);
  digitalWrite(MQTT_INFO_LED, LOW);
  digitalWrite(BLE_INFO_LED, LOW);
}

void checkButton() {
  // check for button press
  if ( digitalRead(RESET_PIN) == HIGH ) {
    delay(50);
    if ( digitalRead(RESET_PIN) == HIGH ) {
      //disconnect from wifi and open config portal
      Serial.println("Button Pressed");
      digitalWrite(WIFI_INFO_LED, LOW);
      digitalWrite(MQTT_INFO_LED, LOW);
      // start portal
      Serial.println("Starting config portal");
      wm.resetSettings();
      Serial.println("Resetting recorded creditentials");
      wm.setConfigPortalTimeout(180);

      if (!wm.startConfigPortal(APName, "academy2021")) {
        Serial.println("failed to connect or hit timeout");
        digitalWrite(WIFI_INFO_LED, LOW);
        delay(3000);
        ESP.restart();
      } else {
        Serial.println("connected");
        digitalWrite(WIFI_INFO_LED, HIGH);
      }
    }
  }
}


void setup() {
  Serial.begin(9600);
  setAPandWiFiName();
  connectToWiFi();
  setupMQTT();
  setupIO();
  BLEScannerSetup();
  //  log_d("Total heap: %d", ESP.getHeapSize());
  //  log_d("Free heap: %d", ESP.getFreeHeap());
  //  log_d("Total PSRAM: %d", ESP.getPsramSize());
  //  log_d("Free PSRAM: %d", ESP.getFreePsram());

}

void reconnectToTheBroker() {
  int numberOfConnectionsTried = 0;
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker..");
    if (mqttClient.connect(CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD)) {
      Serial.println("Connected.");
      digitalWrite(MQTT_INFO_LED, HIGH);
      //subscribe to topic
      //mqttClient.subscribe("/o1/desk1/esp32-1/cm");
    }
    else {
      Serial.print("Connection failed, rc=");
      Serial.print(mqttClient.state());
      digitalWrite(MQTT_INFO_LED, LOW);
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

  BeaconData uniqueBuffer[BUFFER_SIZE];
  int numberOfDevicesFound = 0;
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(WIFI_INFO_LED, HIGH);
    if (!mqttClient.connected()) {
      Serial.println("Reconnecting to the broker..");
      digitalWrite(MQTT_INFO_LED, LOW);
      reconnectToTheBroker();

    }
  }
  else {
    digitalWrite(WIFI_INFO_LED, LOW);
    digitalWrite(MQTT_INFO_LED, LOW);
    connectToWiFi();
  }

  Serial.println("mqttClient.loop()");
  mqttClient.loop();
  Serial.println("checkButton()");
  checkButton();
  //Scan the devices
  Serial.println("BLEScannerLoop()");
  BLEScannerLoop();
  Serial.println("printBuffer()");
  printBuffer(buffer, bufferIndex);

  /* filterBuffer(buffer,uniqueBuffer,bufferIndex,BUFFER_SIZE); Actually should be like this.
    But buffer and bufferIndex is global.*/
  numberOfDevicesFound = filterBuffer(uniqueBuffer, bufferIndex, BUFFER_SIZE);

  Serial.print("Number of devices found: ");
  Serial.print(numberOfDevicesFound);
  printBuffer(uniqueBuffer, numberOfDevicesFound);

  /* -- BLE DEBUG -- */
  char *myMacAdress = "ac:23:3f:a3:35:68";
  int threshold = -200;
  debugIsPeopleInTheRoom(uniqueBuffer, numberOfDevicesFound, myMacAdress, threshold);
  /* -- BLE DEBUG -- */

  /* -- MQTT DEBUG -- */
  // MQTTDebugger(999);
  /* -- MQTT DEBUG -- */

  unsigned long now = millis();
  if (now - last_time > DATA_SEND) {
    if (numberOfDevicesFound >= 0) { //We can either send 0 devices, or dont send data.
      publishScanDataToMQTT(uniqueBuffer, numberOfDevicesFound);
    }

    publishDeviceInfoToMQTT();

    last_time = now;
  }
  bufferIndex = 0; //Reset the buffer.

}

int filterBuffer(BeaconData *filteredBuffer, int localBufferIndex, int bufferSize) {
  Serial.println("Filtering the buffer....");
  int deviceCounted[bufferSize] = {0};
  int numberOfUniqeAdresses = 0;
  int currentRssi = 0;
  int i  = 0; int j = 0; int counter = 0;
  for (j = 0; j < localBufferIndex; j++) {
    if (!deviceCounted[j])  // Only enter inner-loop if MAC address hasn't been counted already
    {
      for (i = 0; i < localBufferIndex; i++) {
        if (strcmp(buffer[i].address, buffer[j].address) == 0) {
          counter++; //How many repetation for that mac adress.
          currentRssi += buffer[i].rssi;
          deviceCounted[i] = 1;  // Mark device as counted
        }
      }
      if(counter>2) {
      numberOfUniqeAdresses++;
      Serial.print("MAC: ");
      Serial.print(buffer[j].address);
      Serial.print(" : COUNTER: ");
      Serial.println(counter);
      strcpy(filteredBuffer[numberOfUniqeAdresses - 1].address, buffer[j].address);
      filteredBuffer[numberOfUniqeAdresses - 1].rssi = currentRssi / counter; } 
      counter = 0; currentRssi = 0;
    }
  }
  return numberOfUniqeAdresses;
}

void printBuffer(BeaconData *printBuffer, int bufferSize) {
  Serial.println("\nPrinting the buffer...");
  for (uint8_t i = 0; i < bufferSize; i++) {
    Serial.print(i);
    Serial.print(" : ");
    Serial.print(printBuffer[i].address);
    Serial.print(" : ");
    Serial.println(printBuffer[i].rssi);
  }
}

void publishScanDataToMQTT(BeaconData *uniqueBuffer, int numberOfDevicesFound) {

  Serial.print("Publishing the data...");
  if (mqttClient.setBufferSize(LOCAL_MQTT_MAX_PACKET_SIZE)) {
    Serial.print("Buffer size set to: "); Serial.println(mqttClient.getBufferSize());
  }
  else {
    Serial.print("Cant set the buffer..:(");
  }
  String payloadString = "{\"e\":[";
  for (uint8_t i = 0; i < numberOfDevicesFound; i++) {
    payloadString += "{\"m\":\"";
    payloadString += String(uniqueBuffer[i].address);
    payloadString += "\",\"r\":";
    payloadString += String(uniqueBuffer[i].rssi);
    payloadString += "}";
    if (i < numberOfDevicesFound - 1) {
      payloadString += ',';
    }
  }
  // SenML ends. Add this stations MAC
  payloadString += "],\"mac\":\"";
  payloadString += String(WiFi.macAddress()); payloadString += "\"}";

  // Print and publish payload
  Serial.print("MAX len: ");
  Serial.println(LOCAL_MQTT_MAX_PACKET_SIZE);

  Serial.print("Payload length: ");
  Serial.println(payloadString.length());
  Serial.println(payloadString);

  uint8_t messageCharBuffer[LOCAL_MQTT_MAX_PACKET_SIZE];
  payloadString.getBytes(messageCharBuffer, payloadString.length() + 1);

  payloadString.getBytes(message_char_buffer, payloadString.length() + 1);
  if (mqttClient.setBufferSize(LOCAL_MQTT_MAX_PACKET_SIZE)) {
    Serial.print("Buffer size set to: "); Serial.println(mqttClient.getBufferSize());
  }
  else {
    Serial.print("Cant set the buffer..:(");
  }
  int result = mqttClient.publish("/scn-dvc", message_char_buffer, payloadString.length(), false);
  Serial.print("PUB Result: ");
  Serial.println(result);
  if (mqttClient.setBufferSize(50)) {
    Serial.print("Buffer size set to: "); Serial.println(mqttClient.getBufferSize());
  }
  else {
    Serial.print("Cant set the buffer..:(");
  }
  // Serial.println(mqttClient.publish("/o1/m1/esp32-1/info/yusuf", "test"));
}

void publishDeviceInfoToMQTT() {
  //Not implemented. Temp and humidity.
  //result = client.publish("/o1/m1/esp32-1/info", message_char_buffer, payloadString.length(), false);
  //  Serial.print("PUB Result: ");
  //  Serial.println(result);
}

void debugIsPeopleInTheRoom(BeaconData *uniqueBuffer, int numberOfDevicesFound, char address[17], int threshold) {
  for (uint8_t i = 0; i < numberOfDevicesFound; i++) {
    if (strcmp(address, uniqueBuffer[i].address) == 0) {
      Serial.println("Requested address is found");
      //-100<-70 dont take it.
      if (uniqueBuffer[i].rssi > threshold) {
        Serial.println("Requested people found in the room.");
        digitalWrite(BLE_INFO_LED, HIGH);
        return;
      }
    }
  }
  Serial.println("Requested people is not in the room.");
  digitalWrite(BLE_INFO_LED, LOW);
  return;
}

void MQTTDebugger(int numberOfChars) {
  Serial.println("Starting the benchmark...\nGet ready RabbitMQ");
  for (uint32_t j = 0; j < numberOfChars; j++) {
    Serial.print("Trying to increasing buffer size...Result: ");
    Serial.println(mqttClient.setBufferSize(numberOfChars + 30)); //minimum 30 is required.

    String payloadString = "S";
    for (uint32_t i = 0; i < j; i++) {
      payloadString += "Y";
    }

    Serial.print("Debugger Payload length: ");
    Serial.println(payloadString.length());

    uint8_t messageCharBuffer[LOCAL_MQTT_MAX_PACKET_SIZE];
    payloadString.getBytes(message_char_buffer, payloadString.length() + 1);
    int result = mqttClient.publish("/debug/yusuf", message_char_buffer, payloadString.length(), false);
    Serial.print("PUB Result: ");
    Serial.println(result);
    delay(100); //So that broker doesnt crash.
  }
  Serial.print("Set buffer size to default...Result: ");
  Serial.println(mqttClient.setBufferSize(256));
}
