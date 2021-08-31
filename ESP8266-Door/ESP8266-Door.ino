#include <Arduino.h>
//This code is for Meeting or Kitchen ESP32s Scanning Gateway
//Topic list included in - https://docs.google.com/document/d/1uNCvFoLJsAC_Qh4L8_6ozjhal6APVGUYTeoTuyXoGE8/edit?usp=sharing

//Device MAC Address: F4:CF:A2:EF:B9:9E

//MQTT and WiFi
//#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

#include "NeoPixel.h"
#define NEOPIXEL_PIN 13 //D7
#define PIXEL_COUNT 24
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
NeoPixel DoorLight;

//WiFi reset pin
#define RESET_PIN 12 //d6

//LED for debug
#define WIFI_INFO_LED 4 //d2
//If we are using Onboard leds we cant connect anything to D0 and D4/
#define ONBOARD_LED1 16 //Reversed LOW = HIGH //D0 //Near the power
#define ONBOARD_LED2 2 //Reversed LOW = HIGH //D4 //Near the antenna

#define DATA_SEND_DELAY 5000 //Per miliseconds
#define MQTT_MAX_PACKET_SIZE 1000
//#define ONBOARD_LED 2 //Onboard LED used for debugging.

//Callback
#define NUMBER_OF_STRING 4
#define MAX_STRING_SIZE 40

//Change Config File to Connect the MQTT Broker
#include "MQTT_Config.h"

unsigned long lastSendTime = 0;


//WiFiClientSecure wifiClient;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiManager wm;

uint8_t message_char_buffer[MQTT_MAX_PACKET_SIZE];
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

/*void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ONBOARD_LED, LOW);
    delay(500);
    digitalWrite(ONBOARD_LED, HIGH);
  }
  Serial.print("Connected to the WiFi.");
  digitalWrite(ONBOARD_LED, LOW);
  }*/

void connectToWiFi() {
  /* char APName[28];
    String getAPName = "ESP8266 - ";
    getAPName += WiFi.macAddress() ;
    strcpy(APName, getAPName.c_str());*/
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) {
    wm.setConnectTimeout(300);
    wm.setConfigPortalTimeout(120);
    bool res = wm.autoConnect(APName, "academy2021");
    if (!res) {
      digitalWrite(WIFI_INFO_LED, LOW);
      Serial.println("Failed to connect");
      ESP.restart;
    }
    else {
      digitalWrite(WIFI_INFO_LED, HIGH);
      Serial.println("connected to wifi");
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {


  //strcpy(DeviceMacAdress,WiFi.macAddress());

  digitalWrite(ONBOARD_LED2, LOW);
  delay(500);
  digitalWrite(ONBOARD_LED2, HIGH);
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
  /*  char deviceMacAddress[18];
    String getMAC = WiFi.macAddress();
    strcpy(deviceMacAddress, getMAC.c_str());*/

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
  setAPandWiFiName();
  connectToWiFi();
  setupMQTT();
  DoorLight.setupNeoPixel();
  setupIO();
}

void setupIO() {
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(WIFI_INFO_LED, OUTPUT);
  pinMode(ONBOARD_LED1, OUTPUT);
  pinMode(ONBOARD_LED2, OUTPUT);
  digitalWrite(WIFI_INFO_LED, LOW);
  digitalWrite(ONBOARD_LED1, HIGH);
  digitalWrite(ONBOARD_LED2, HIGH);
}
void checkButton() {
  // check for button press
  if ( digitalRead(RESET_PIN) == HIGH ) {
    delay(50);
    if ( digitalRead(RESET_PIN) == HIGH ) {
      //disconnect from wifi and open config portal
      Serial.println("Button Pressed");
      digitalWrite(WIFI_INFO_LED, LOW);
      digitalWrite(ONBOARD_LED1, HIGH);
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

void reconnectToTheBroker() {
  int numberOfConnectionsTried = 0;
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker..");
    if (mqttClient.connect(CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD)) {
      Serial.println("Connected.");
      digitalWrite(ONBOARD_LED1, LOW);
      //subscribe to topic
      mqttClient.subscribe("/nrom");
      mqttClient.subscribe("/next-event/yusuf");
    }
    else {
      Serial.print("Connection failed, rc=");
      Serial.print(mqttClient.state());
      digitalWrite(ONBOARD_LED1, HIGH);
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

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(WIFI_INFO_LED, HIGH);
    if (!mqttClient.connected()) {
      Serial.println("Reconnecting to the broker..");
      digitalWrite(ONBOARD_LED1, HIGH);
      reconnectToTheBroker();

    }
  }
  else {
    digitalWrite(WIFI_INFO_LED, LOW);
    digitalWrite(ONBOARD_LED1, HIGH);
    connectToWiFi();
  }
  mqttClient.loop();
  checkButton();

  unsigned long currentMillis = millis();
  if (currentMillis - lastSendTime > DATA_SEND_DELAY) {
    //Serial.print("Device Mac Address: ");
    //Serial.println(WiFi.macAddress());
    publishScanDataToMQTT();
    publishDeviceInfoToMQTT();
    lastSendTime = currentMillis;
  }

}


void publishScanDataToMQTT() {
  Serial.print("PUB Result: ");
  Serial.println(mqttClient.publish("/test-data", "test-esp8266"));
}

void publishDeviceInfoToMQTT() {
  //Not implemented. Temp and humidity.
}

/*int readTempAndHumidity(float *humidity, float *temperature) {
   humidity = dht.readHumidity();
   temperature = dht.readTemperature();
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
  }*/
