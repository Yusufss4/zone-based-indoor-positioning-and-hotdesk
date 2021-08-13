#include <Arduino.h>

//MQTT and WiFi
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

//Bluetooth Scan
#include "BLEScanner.h"


#define DATA_SEND 60000
//Change Config File to Connect the MQTT Broker and WiFi
#include "MQTT_Config.h"

long last_time = 0;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void connectToWiFi() {
  Serial.print("Connecting to..");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected to the WiFi.");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print("\n");
}

void setupMQTT() {
  wifiClient.setCACert(ca_cert);
  mqttClient.setServer(MQTT_SERVER_NAME, MQTT_PORT);
  // set the callback function
  mqttClient.setCallback(callback);
  mqttClient.setKeepAlive(60);
}


void setup() {
  Serial.begin(9600);
  connectToWiFi();
  setupMQTT();
  BLEScannerSetup();
  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());
}

void reconnectToTheBroker() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker..");
    if (mqttClient.connect(CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD)) {
      Serial.println("Connected.");
      // subscribe to topic
      mqttClient.subscribe("/o1/desk1/esp32-1/cm");
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
  long now = millis();
  if (now - last_time > DATA_SEND) {

    // Publishing data through MQTT
    
    //publish byte.
    //int somethingElse = 123;
    //byte RSSIValue[] = {0x82}; //130
    //RSSIValue[0] = 123;
    //sprintf(data, "%d", humidity);
    //Serial.write(RSSIValue[0]);
    //mqttClient.publish("/o1/m1/esp32-1/scn-dvc", RSSIValue, 1); //1 byte.

    char RSSI_1[] = "123";
    //sprintf(data, "%f", pres);
    Serial.println(RSSI_1);
    mqttClient.publish("/o1/m1/esp32-1/scn-dvc", RSSI_1);
    
    //publish string
    char MACAdress[] = "Yusuf";
    //sprintf(data, "%f", pres);
    Serial.println(MACAdress);
    ////o1/m1/esp32-1/info
    mqttClient.publish("/o1/m1/esp32-1/info", MACAdress);
    last_time = now;
  }

}
