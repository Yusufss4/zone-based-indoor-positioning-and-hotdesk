#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define DATA_SEND 60000
#include "MQTT_Config.h"

long last_time = 0;
//char data[100];

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void connectToWiFi() {

  Serial.print("Connecting to..");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(WIFI_SSID);

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
      mqttClient.subscribe("/home/esp32s/yusuf/commands");
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
  long now = millis();
  if (now - last_time > DATA_SEND) {

    // Send data
//    float temp = (float)random(0, 50); //bme.readTemperature();
//    float hum = (float)random(0, 100); //bme.readHumidity();
//    float pres = (float)random(0, 30); //bme.readPressure() / 100;


    // Publishing data through MQTT
    
    //publish byte in terms of integer.
    //uint8_t RSSIValue = 44;
    //RSSIValue = 38;
    //sprintf(data, "%d", RSSIValue);
    //Serial.println(RSSIValue);
    //mqttClient.publish("/home/esp32s/yusuf/temp", RSSIValue, 1);

    //publish byte.
    int somethingElse = 123;
    byte humidity[] = {0x82}; //130
    humidity[0] = 123;
    //sprintf(data, "%d", humidity);
    log_d("Humidity: \n");
    Serial.write(humidity[0]);
    mqttClient.publish("/home/esp32s/yusuf/humidity", humidity, 1); //1 byte.
    
    //publish string
    char MACAdress[30] = "00:1B:44:11:3A:B7";
    //sprintf(data, "%f", pres);
    log_d("MAC Adres: \n");
    Serial.println(MACAdress);
    mqttClient.publish("/home/esp32s/yusuf/pressure", MACAdress);
    last_time = now;
  }

}
