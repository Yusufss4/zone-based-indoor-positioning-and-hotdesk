//ESP32 - Scanner LED KIT
#define WIFI_LED 12 //12
#define MQTT_LED 14 //14
#define BLE_LED 27 //27

// The pin the LED is connected to
void setup() {
  Serial.begin(9600);
  pinMode(WIFI_LED, OUTPUT);
  pinMode(MQTT_LED, OUTPUT);
  pinMode(BLE_LED, OUTPUT);// Declare the LED as an output
    digitalWrite(WIFI_LED,LOW);
  digitalWrite(MQTT_LED,LOW);
  digitalWrite(BLE_LED,LOW);
}

void loop() {
  Serial.println("Led Test...");
  delay(1000);
  digitalWrite(WIFI_LED,LOW);
  digitalWrite(MQTT_LED,LOW);
  digitalWrite(BLE_LED,LOW);
  delay(1000);
  digitalWrite(WIFI_LED,HIGH);
  digitalWrite(MQTT_LED,HIGH);
  digitalWrite(BLE_LED,HIGH); // Turn the LED on
}
