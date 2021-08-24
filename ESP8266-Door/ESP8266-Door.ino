#include <Arduino.h>
//This code is for Meeting or Kitchen ESP32s Scanning Gateway
//Topic list included in - https://docs.google.com/document/d/1uNCvFoLJsAC_Qh4L8_6ozjhal6APVGUYTeoTuyXoGE8/edit?usp=sharing

//Device MAC Address: F4:CF:A2:EF:B9:9E

//MQTT and WiFi
//#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "NeoPixel.h"
#define NEOPIXEL_PIN 13
#define PIXEL_COUNT 12
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
NeoPixel DoorLight;

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
  char messageMacAddress[17];
  strcpy(messageMacAddress, strtok(messageTemp, delimeter));
  char deviceMacAddress[18];
  String getMAC = WiFi.macAddress();
  strcpy(deviceMacAddress, getMAC.c_str());

  char UUID[32];
  char eventStatus[10];
  char eventTime[6];
  int numOfPeopleInTheRoom = 0;
  int capacityOfRoom = 0;

  if (strcmp(messageMacAddress, deviceMacAddress) == 0) {
    // Check if the MQTT message was received on topic esp32/relay1
    if (strcmp(topic, "/nrom/yusuf") == 0) {
      Serial.println("Detected message at the topic nrom");
      numOfPeopleInTheRoom = atoi(strtok(NULL, delimeter));
      capacityOfRoom = atoi(strtok(NULL, delimeter));

      uint32_t orange = strip.Color(255, 69, 0);
      uint32_t black = strip.Color(0, 0, 0);
      uint32_t blue = strip.Color(0, 0, 255);
      uint32_t red = strip.Color(255, 0, 0);
      //changeLedStatusCalculateCapacity(orange, black, red, numOfPeopleInTheRoom, capacityOfRoom);
      DoorLight.changeLedStatusOneLedPerPerson(orange, black, red, numOfPeopleInTheRoom, capacityOfRoom);

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
  Serial.begin(9600);
  connectToWiFi();
  setupMQTT();
  DoorLight.setupNeoPixel();
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
    //Serial.print("Device Mac Address: ");
    //Serial.println(WiFi.macAddress());
    publishScanDataToMQTT();
    publishDeviceInfoToMQTT();
    last_time = now;
  }

}


void publishScanDataToMQTT() {
  Serial.print("PUB Result: ");
  Serial.println(mqttClient.publish("/o1/m1/esp32-1/info", "test-esp8266"));
}

void publishDeviceInfoToMQTT() {
  //Not implemented. Temp and humidity.
}
