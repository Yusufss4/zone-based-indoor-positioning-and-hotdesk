#include <Arduino.h>
//This code is for Meeting or Kitchen ESP32s Scanning Gateway
//Topic list included in - https://docs.google.com/document/d/1ixmkAzqzE-u8imwFLdVT4XZ1hFTCaz8fN4PDlNe0_3w/edit?usp=sharing

//MQTT and WiFi
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

//Bluetooth Scan
#include "BLEScanner.h"
#include "Globals.h"

#define DATA_SEND 5000 //Per miliseconds
#define MQTT_MAX_PACKET_SIZE 1000

//Change Config File to Connect the MQTT Broker and WiFi
#include "MQTT_Config.h"

unsigned long last_time = 0;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

uint8_t bufferIndex = 0; // Found devices counter
BeaconData buffer[BUFFER_SIZE];
uint8_t message_char_buffer[MQTT_MAX_PACKET_SIZE];


void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("Connected to the WiFi.");
}

#ifndef ESP32-SCANNER
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
  wifiClient.setCACert(ca_cert);
  mqttClient.setServer(MQTT_SERVER_NAME, MQTT_PORT);
  // set the callback function
#ifndef ESP32-SCANNER
  mqttClient.setCallback(callback);
#endif
  mqttClient.setKeepAlive(60);
}


void setup() {
  Serial.begin(9600);
  connectToWiFi();
  setupMQTT();
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
      //subscribe to topic
      //mqttClient.subscribe("/o1/desk1/esp32-1/cm");
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

  BeaconData uniqueBuffer[BUFFER_SIZE];
  int numberOfDevicesFound = 0;

  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to the broker..");
    reconnectToTheBroker();
  }

  mqttClient.loop();
  //Scan the devices
  BLEScannerLoop();
  printBuffer(buffer, bufferIndex);

  /* filterBuffer(buffer,uniqueBuffer,bufferIndex,BUFFER_SIZE); Actually should be like this.
    But buffer and bufferIndex is global.*/
  numberOfDevicesFound = filterBuffer(uniqueBuffer, bufferIndex, BUFFER_SIZE);

  Serial.print("Number of devices found: ");
  Serial.print(numberOfDevicesFound);
  printBuffer(uniqueBuffer, numberOfDevicesFound);


  unsigned long now = millis();
  if (now - last_time > DATA_SEND) {

    publishScanDataToMQTT(uniqueBuffer, numberOfDevicesFound);
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
      numberOfUniqeAdresses++;
      Serial.print("MAC: ");
      Serial.print(buffer[j].address);
      Serial.print(" : COUNTER: ");
      Serial.println(counter);
      strcpy(filteredBuffer[numberOfUniqeAdresses - 1].address, buffer[j].address);
      filteredBuffer[numberOfUniqeAdresses - 1].rssi = currentRssi / counter;
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

  String payloadString = "{\"e\":[";
  Serial.println("Publishing the data...");
  for (uint8_t i = 0; i < numberOfDevicesFound; i++) {
    payloadString += "{\"m\":\"";
    payloadString += String(uniqueBuffer[i].address);
    payloadString += "\",\"r\":\"";
    payloadString += String(uniqueBuffer[i].rssi);
    payloadString += "\"}";
    if (i < numberOfDevicesFound - 1) {
      payloadString += ',';
    }
  }
  // SenML ends. Add this stations MAC
  payloadString += "]\"}";
  //    payloadString += "],\"st\":\"";
  //    payloadString += String(WiFi.macAddress());
  //    // Add board temperature in fahrenheit
  //    payloadString += "\",\"t\":\"";
  //    payloadString += String(40);
  //    payloadString += "\"}";

  // Print and publish payload
  Serial.print("MAX len: ");
  Serial.println(MQTT_MAX_PACKET_SIZE);

  Serial.print("Payload length: ");
  Serial.println(payloadString.length());
  Serial.println(payloadString);

  uint8_t messageCharBuffer[MQTT_MAX_PACKET_SIZE];
  payloadString.getBytes(messageCharBuffer, payloadString.length() + 1);

  payloadString.getBytes(message_char_buffer, payloadString.length() + 1);
  //int result = mqttClient.publish("/o1/m1/esp32-1/scn-dvc", message_char_buffer, payloadString.length(), false);
  //Serial.print("PUB Result: ");
  //Serial.println(result);
  mqttClient.publish("/o1/m1/esp32-1/info", "test");
}

void publishDeviceInfoToMQTT() {
  //Not implemented. Temp and humidity.
  //result = client.publish("/o1/m1/esp32-1/info", message_char_buffer, payloadString.length(), false);
  //  Serial.print("PUB Result: ");
  //  Serial.println(result);
}
