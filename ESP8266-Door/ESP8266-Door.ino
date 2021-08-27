#include <Arduino.h>
//This code is for Meeting or Kitchen ESP32s Scanning Gateway
//Topic list included in - https://docs.google.com/document/d/1uNCvFoLJsAC_Qh4L8_6ozjhal6APVGUYTeoTuyXoGE8/edit?usp=sharing

//Device MAC Address: F4:CF:A2:EF:B9:9E

//MQTT and WiFi
//#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "NeoPixel.h"
#define NEOPIXEL_PIN 13 //D7
#define PIXEL_COUNT 24
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
NeoPixel DoorLight;

#define DATA_SEND_DELAY 5000 //Per miliseconds
#define MQTT_MAX_PACKET_SIZE 1000
#define ONBOARD_LED 2 //Onboard LED used for debugging.

//Callback
#define NUMBER_OF_STRING 4
#define MAX_STRING_SIZE 40

//Change Config File to Connect the MQTT Broker and WiFi
#include "MQTT_Config.h"

unsigned long lastSendTime = 0;


//WiFiClientSecure wifiClient;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//Temp and Humidity
#include <DHT.h>
#define DHTPIN 15 //D8
#define DHTTYPE DHT11
#define SAMPLE_DELAY 10000
DHT dht(DHTPIN, DHTTYPE);
unsigned long lastSampleTime = 0;

uint8_t message_char_buffer[MQTT_MAX_PACKET_SIZE];


void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ONBOARD_LED, LOW);
    delay(500);
    digitalWrite(ONBOARD_LED, HIGH);
  }
  Serial.print("Connected to the WiFi."); 
  digitalWrite(ONBOARD_LED, LOW);
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
    if (strcmp(topic, "/nrom") == 0) {
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
  dht.begin();
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
}

void reconnectToTheBroker() {
  int numberOfConnectionsTried = 0;
  while (!mqttClient.connected()) {
    digitalWrite(ONBOARD_LED, LOW);
    Serial.println("Reconnecting to MQTT Broker..");
    if (mqttClient.connect(CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD)) {
      Serial.println("Connected.");
      //subscribe to topic
      mqttClient.subscribe("/nrom");
      mqttClient.subscribe("/next-event/yusuf");
      digitalWrite(ONBOARD_LED, LOW);
    }
    else {
      Serial.print("Connection failed, rc=");
      Serial.print(mqttClient.state());
      numberOfConnectionsTried++;
      if (numberOfConnectionsTried > 5) {
        Serial.print("Rebooting the device...");
        ESP.restart();
      }
      digitalWrite(ONBOARD_LED, HIGH);
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

  unsigned long currentMillis = millis();
  if (currentMillis - lastSendTime > DATA_SEND_DELAY) {
    //Serial.print("Device Mac Address: ");
    //Serial.println(WiFi.macAddress());
    publishScanDataToMQTT();
    publishDeviceInfoToMQTT();
    lastSendTime = currentMillis;
  }

  if (currentMillis - lastSampleTime >= SAMPLE_DELAY) {
    float humidity = 0;
    float temperature = 0;
    if (readTempAndHumidity(&humidity, &temperature) == 1) {
      publishTempAndHumidityDataToMQTT(&humidity, &temperature);
    }
    else
    {
      Serial.println(F("Failed to read from DHT sensor!"));
    }
    lastSampleTime = currentMillis;
  }
}


void publishScanDataToMQTT() {
  Serial.print("PUB Result: ");
  Serial.println(mqttClient.publish("/test-data", "test-esp8266"));
}

void publishDeviceInfoToMQTT() {
  //Not implemented. Temp and humidity.
}

int readTempAndHumidity(float *humidity, float *temperature) {
  *humidity = dht.readHumidity();
  *temperature = dht.readTemperature();
  if (isnan(*humidity) || isnan(*temperature)) {
    Serial.println(F("Failed to read from DHT sensor!!"));
    return -1;
  }
  else {
    return 1;
  }
}

void publishTempAndHumidityDataToMQTT(float *humidity, float *temperature) {
  int result = 0;
  String payload = "{ \"mac\":\"";
  payload += WiFi.macAddress();
  payload += "\",\"temp\":";
  payload += String(*temperature).c_str();
  payload += ",\"hmd\":";
  payload += String(*humidity).c_str();
  payload += "}";

  //convert string to byte and publish
  uint8_t messageCharBuffer[MQTT_MAX_PACKET_SIZE];
  payload.getBytes(messageCharBuffer, payload.length() + 1);
  result = mqttClient.publish("/temp-hmd", messageCharBuffer, payload.length(), false);
}
