#include <Arduino.h>
//This code is for Meeting or Kitchen ESP32s Scanning Gateway
//Topic list included in - https://docs.google.com/document/d/1ixmkAzqzE-u8imwFLdVT4XZ1hFTCaz8fN4PDlNe0_3w/edit?usp=sharing

//MQTT and WiFi
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

//Bluetooth Scan
#include "BLEScanner.h"

#define DATA_SEND 20000

//Change Config File to Connect the MQTT Broker and WiFi
#include "MQTT_Config.h"

unsigned long last_time = 0;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
char data[50];

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
    }
  }
}


void loop() {

  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to the broker..");
    reconnectToTheBroker();
  }

  mqttClient.loop();
  BLEScannerLoop();
  unsigned long now = millis();
  if (now - last_time > DATA_SEND) {

    // Publishing data through MQTT

    //publish byte.
    //int somethingElse = 123;
    //byte RSSIValue[] = {0x82}; //130
    //RSSIValue[0] = 123;
    //sprintf(data, "%d", humidity);
    //Serial.write(RSSIValue[0]);
    //mqttClient.publish("/o1/m1/esp32-1/scn-dvc", RSSIValue, 1); //1 byte.

    //char RSSI_1[]
    int RSSI_1 = random(10, 80);
    sprintf(data, "%d", RSSI_1);
    //char RSSI_1[] = "123";
    //sprintf(data, "%f", pres);
    mqttClient.publish("/o1/m1/esp32-1/scn-dvc", data);

    //publish string
    char MACAdress[] = "00:1B:44:11:3A:B7";
    //sprintf(data, "%f", pres);;
    mqttClient.publish("/o1/m1/esp32-1/info", MACAdress);
    last_time = now;
  }

}
