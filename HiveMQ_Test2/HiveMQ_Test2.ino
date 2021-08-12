#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>


const char *SSID = "SUPERONLINE_WiFi_5918";
const char *PWD = "V9dp6ACGgxjE";

const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n" \
"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n" \
"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n" \
"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n" \
"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n" \
"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n" \
"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n" \
"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n" \
"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n" \
"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n" \
"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n" \
"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n" \
"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n" \
"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n" \
"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n" \
"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n" \
"-----END CERTIFICATE-----\n";



long last_time = 0;
char data[100];

// MQTT client
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient); 

char *mqttServer = "f1ec066d96ce468e9e8ded72f52952b9.s2.eu.hivemq.cloud";
int mqttPort = 8883;


void connectToWiFi() {
  
  Serial.print("Connectiog to ");
 
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("Connected.");
  
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
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
  mqttClient.setKeepAlive(60);
}


void setup() {
  Serial.begin(9600);
  connectToWiFi();
  setupMQTT();
  
    
}

void reconnectToTheBroker() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str(),"yusufss","Yusuf123*")) {
        Serial.println("Connected.");
        // subscribe to topic
        mqttClient.subscribe("/home/esp32s/yusuf/commands");
      }
      else {
      Serial.print("Connection failed, rc=");
      Serial.print(mqttClient.state()); }
      
      
  }
}


void loop() {

      if (!mqttClient.connected()) {
    reconnectToTheBroker(); }

  mqttClient.loop();
  long now = millis();
  if (now - last_time > 60000) {

    // Send data
    float temp = (float)random(0,50); //bme.readTemperature();
    float hum = (float)random(0, 100); //bme.readHumidity();
    float pres = (float)random(0, 30); //bme.readPressure() / 100;
    

    // Publishing data throgh MQTT
    sprintf(data, "%f", temp);
    Serial.println(data);
    mqttClient.publish("/home/esp32s/yusuf/temp", data);
    sprintf(data, "%f", hum);
    Serial.println(hum);
    mqttClient.publish("/home/esp32s/yusuf/humidity", data);
    sprintf(data, "%f", pres);
    Serial.println(pres);
    mqttClient.publish("/home/esp32s/yusuf/pressure", data);
    last_time = now;
  }

}
