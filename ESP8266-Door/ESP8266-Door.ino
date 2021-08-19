#include <Arduino.h>
//This code is for Meeting or Kitchen ESP32s Scanning Gateway
//Topic list included in - https://docs.google.com/document/d/1ixmkAzqzE-u8imwFLdVT4XZ1hFTCaz8fN4PDlNe0_3w/edit?usp=sharing

//MQTT and WiFi
//#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Bluetooth Scan

#define DATA_SEND 5000 //Per miliseconds
#define MQTT_MAX_PACKET_SIZE 1000

//Callback
#define NUMBER_OF_STRING 4
#define MAX_STRING_SIZE 40

//Change Config File to Connect the MQTT Broker and WiFi
#include "MQTT_Config.h"

unsigned long last_time = 0;

//WiFiClientSecure wifiClient;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

uint8_t message_char_buffer[MQTT_MAX_PACKET_SIZE];


void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("Connected to the WiFi.");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  //String messageTemp;
  char messageTemp[MAX_STRING_SIZE*NUMBER_OF_STRING];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    //messageTemp += (char)payload[i];
    //strcat(messageTemp,(char)payload[i]);
    messageTemp[i] = payload[i];
  }
  messageTemp[length] = '\0';
  Serial.print("\n");

 // Check if the MQTT message was received on topic esp32/relay1
 if (strcmp(topic, "/nrom/yusuf") == 0) {
  Serial.println("Detected message at the topic nrom");
  
 }
 if (strcmp(topic, "/next-event/yusuf") == 0) {
   Serial.println("Detected message at the topic next-event");
 }
 Serial.println(messageTemp);
}

void setupMQTT() {
  //wifiClient.setCACert(ca_cert);
  mqttClient.setServer(MQTT_SERVER_NAME, MQTT_PORT);
  // set the callback function
  mqttClient.setCallback(callback);
  mqttClient.setKeepAlive(60);
}


void setup() {
  Serial.begin(9600);
  connectToWiFi();
  setupMQTT();
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
      mqttClient.subscribe("/nrom/yusuf");
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


  unsigned long now = millis();
  if (now - last_time > DATA_SEND) {
    publishScanDataToMQTT();
    publishDeviceInfoToMQTT();
    last_time = now;
  }

}


void publishScanDataToMQTT() {

//  String payloadString = "{\"e\":[";
//  Serial.println("Publishing the data...");
//  for (uint8_t i = 0; i < numberOfDevicesFound; i++) {
//    payloadString += "{\"m\":\"";
//    payloadString += String(uniqueBuffer[i].address);
//    payloadString += "\",\"r\":\"";
//    payloadString += String(uniqueBuffer[i].rssi);
//    payloadString += "\"}";
//    if (i < numberOfDevicesFound - 1) {
//      payloadString += ',';
//    }
//  }
//  // SenML ends. Add this stations MAC
//  payloadString += "]\"}";
//  //    payloadString += "],\"st\":\"";
//  //    payloadString += String(WiFi.macAddress());
//  //    // Add board temperature in fahrenheit
//  //    payloadString += "\",\"t\":\"";
//  //    payloadString += String(40);
//  //    payloadString += "\"}";
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
//  int result = mqttClient.publish("/o1/m1/esp32-1/scn-dvc", message_char_buffer, payloadString.length(), false);
//  Serial.print("PUB Result: ");
//  Serial.println(result);
  Serial.print("PUB Result: ");
  Serial.println(mqttClient.publish("/o1/m1/esp32-1/info", "test-esp8266"));
}

void publishDeviceInfoToMQTT() {
  //Not implemented. Temp and humidity.
  //result = client.publish("/o1/m1/esp32-1/info", message_char_buffer, payloadString.length(), false);
  //  Serial.print("PUB Result: ");
  //  Serial.println(result);
}
