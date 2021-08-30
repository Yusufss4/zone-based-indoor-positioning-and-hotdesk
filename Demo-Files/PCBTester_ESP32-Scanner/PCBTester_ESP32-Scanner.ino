//ESP32 - Scanner LED KIT
#define WIFI_LED 12 //12 - BLUE First Led
#define MQTT_LED 14 //14 - GREEN Second Led
#define BLE_LED 27 //27 - YELLOW Third Led 
#define BUTTON_PIN  26 //26

boolean oldState = HIGH;
int     mode     = 0;

void setup() {
  Serial.begin(9600);
  //TODO: Add function for these button startups.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(WIFI_LED, OUTPUT);
  pinMode(MQTT_LED, OUTPUT);
  pinMode(BLE_LED, OUTPUT);// Declare the LED as an output
  digitalWrite(WIFI_LED, LOW);
  digitalWrite(MQTT_LED, LOW);
  digitalWrite(BLE_LED, LOW);
}

void loop() {
  // Get current button state.
  boolean newState = digitalRead(BUTTON_PIN);

  // Check if state changed from high to low (button press).
  if ((newState == LOW) && (oldState == HIGH)) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == LOW) {     // Yes, still low
      if (++mode > 2) mode = 0; // Advance to next mode, wrap around after #8
      switch (mode) {          // Start the new animation...
        case 0:
          digitalWrite(WIFI_LED, HIGH);
          digitalWrite(MQTT_LED, LOW);
          digitalWrite(BLE_LED, LOW);
          Serial.println("Case 0");
          // Black/off
          break;
        case 1:
          digitalWrite(WIFI_LED, LOW);
          digitalWrite(MQTT_LED, HIGH);
          digitalWrite(BLE_LED, LOW);   // Red
          Serial.println("Case 1");
          break;
        case 2:
          digitalWrite(WIFI_LED, LOW);
          digitalWrite(MQTT_LED, LOW);
          digitalWrite(BLE_LED, HIGH);  // Green
          Serial.println("Case 2");
          break;
      }
    }
  }

  // Set the last-read button state to the old state.
  oldState = newState;
}
