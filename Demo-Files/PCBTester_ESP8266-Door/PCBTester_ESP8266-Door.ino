//ESP32 - Door LED KIT
#define WIFI_LED 5 //D1
#define MQTT_LED 4 //D2
#define RIGHTDEVICE_LED 0 //D3
#define BUTTON_PIN 12 //D6

/*If we are using Onboard leds we cant connect anything to D0 and D4*/
#define ONBOARD_LED1 16 //Reversed LOW = HIGH //D0 //Near the power
#define ONBOARD_LED2 2 //Reversed LOW = HIGH //D4 //Near the antenna

int buttonState = 0;
// The pin the LED is connected to
void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  
  pinMode(WIFI_LED, OUTPUT);
  pinMode(MQTT_LED, OUTPUT);
  pinMode(RIGHTDEVICE_LED, OUTPUT);
  pinMode(ONBOARD_LED1,OUTPUT);
  pinMode(ONBOARD_LED2,OUTPUT); // Declare the LED as an output
  digitalWrite(WIFI_LED,LOW);
  digitalWrite(MQTT_LED,LOW);
  digitalWrite(RIGHTDEVICE_LED,LOW);
  digitalWrite(ONBOARD_LED1,HIGH);
  digitalWrite(ONBOARD_LED2,HIGH);
}

void loop() {
//  Serial.println("Led Test...");
//  delay(1000);
//  digitalWrite(WIFI_LED,LOW);
//  digitalWrite(MQTT_LED,LOW);
//  digitalWrite(RIGHTDEVICE_LED,LOW);
//  delay(1000);
//  digitalWrite(WIFI_LED,HIGH);
//  digitalWrite(MQTT_LED,HIGH);
//  digitalWrite(RIGHTDEVICE_LED,HIGH); // Turn the LED on

  buttonState = digitalRead(BUTTON_PIN);
  Serial.println(buttonState);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH
  if (buttonState == HIGH) {
    // turn LED on
    digitalWrite(ONBOARD_LED1, HIGH); 
    digitalWrite(ONBOARD_LED2, LOW);
  } else {
    // turn LED off
    digitalWrite(ONBOARD_LED1, LOW); 
    digitalWrite(ONBOARD_LED2, HIGH);
  }

  
}
