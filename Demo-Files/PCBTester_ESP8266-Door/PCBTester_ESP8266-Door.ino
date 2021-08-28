//ESP32 - Scanner LED KIT
#define WIFI_LED 5 //D1
#define MQTT_LED 4 //D2
#define RIGHTDEVICE_LED 0 //D3

// The pin the LED is connected to
void setup() {
  Serial.begin(9600);
  pinMode(WIFI_LED, OUTPUT);
  pinMode(MQTT_LED, OUTPUT);
  pinMode(RIGHTDEVICE_LED, OUTPUT);// Declare the LED as an output
    digitalWrite(WIFI_LED,LOW);
  digitalWrite(MQTT_LED,LOW);
  digitalWrite(RIGHTDEVICE_LED,LOW);
}

void loop() {
  Serial.println("Led Test...");
  delay(1000);
  digitalWrite(WIFI_LED,LOW);
  digitalWrite(MQTT_LED,LOW);
  digitalWrite(RIGHTDEVICE_LED,LOW);
  delay(1000);
  digitalWrite(WIFI_LED,HIGH);
  digitalWrite(MQTT_LED,HIGH);
  digitalWrite(RIGHTDEVICE_LED,HIGH); // Turn the LED on
}
