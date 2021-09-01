//Change Config File to Connect the MQTT Broker and WiFi
#include "Arduino.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>
#include "MQTT_Config.h"
#include "BLEDevice.h"
#include <map>

#define NUMBER_OF_STRING 6
#define MAX_STRING_SIZE 40

#define MQTT_LED 2
#define BLE_LED 3

#define DATA_SEND 5000 //Per miliseconds
#define MQTT_MAX_PACKET_SIZE 1000

#define LED 13

extern bool loopTaskWDTEnabled;
static TaskHandle_t htask;
static SemaphoreHandle_t barrier;

// Map
std::map<char*, char*> device;

// WiFi
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
bool transmit_flag = false;

//BLE
BLEClient *pClient;
BLERemoteService *pRemoteService;

bool topic_flag = false; //0 = smartdesk / 1 = smartroom

typedef struct message
{
  char device_uuid_val[37];
  char service_uuid_val[37];
  char char_uuid_val[37];
  char employee_id_val[37];
} message_t;

typedef struct event
{
  char device_uuid_val[37];
  char service_uuid_val[37];
  char char_uuid_val[37];
  char event_status_val[37];
  char event_time_val[37];
} event_t;

// Create the struct
message_t msg;
event_t evt;

// MQTT Callback Data
static char messageMacAddress[18];
static char deviceMacAddress[18];

// The remote service we wish to connect to.
//0000180a-0000-1000-8000-00805f9b34fb
//e54b0001-67f5-479e-8711-b3b99198ce6c
//A9A5941D-1681-14E8-E243-78685AB7D125
//E54B0001-67F5-479E-8711-B3B99198CE6C
//e0:5a:5a:c8:36:ac
//3c:71:bf:f5:5d:58

BLEUUID device_uuid("");
BLEUUID service_uuid("");
BLEUUID char_uuid("");

bool doConnect = false;
bool connected = false;
BLERemoteCharacteristic *pRemoteCharacteristic;
BLEAdvertisedDevice *myDevice;

void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char *)pData);
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
  }

  void onDisconnect(BLEClient *pclient)
  {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer()
{
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  //Serial.println(" - Created client");
  pClient = BLEDevice::createClient();

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  pRemoteService = pClient->getService(service_uuid);
  //BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(service_uuid.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(char_uuid);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(char_uuid.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  // Read the value of the characteristic.
  if (pRemoteCharacteristic->canRead())
  {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}

/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  /**
        Called for each advertising BLE server.
    */
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    // We have to check the name string in order to avoid refreshing.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(device_uuid))
    {
      if (control_refresh())
      {
        Serial.print("===Found ESL with matching device UUID==");
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        Serial.print("doConnect = true");
      }
      else
        Serial.println("Name string is same with the previous one, don't update the screen");
    } // Found our server
  }   // onResult
};    // MyAdvertisedDeviceCallbacks

/* 
  1- Create a map
  2- If the UUID value isn't saved before, save it and pair with name string
  3- If the UUID value is saved before, compare its name string with new data.
  4- If it returns equal, return false 
  5- If it doesn't return equal, return true

*/
bool control_refresh()
{
  static char *name_val;
  // If the UUID value isn't saved before, save it and pair with name string
  if (device.find(msg.device_uuid_val) == device.end())
  {
    //TODO: Create a buffer for UUID and name data.
    Serial.println("Control refresh: Saved UUID value, paired with name string");
    name_val = msg.employee_id_val;
    device.insert(std::pair<char*, char*>(msg.device_uuid_val, name_val));
  }
  // If the UUID value is saved before, compare its name string with new data.
  else
  {
    Serial.println("Control refresh: UUID value saved before, comparing with new string");
    // If it returns equal, return false
    if (device.find(msg.device_uuid_val)->second == msg.employee_id_val)
    {
      Serial.println("Control Refresh: Map name");
      Serial.println(device.find(msg.device_uuid_val)->second);
      Serial.println("Control Refresh: new data name");
      Serial.println(msg.employee_id_val);
      Serial.println("Control refresh: they are equal, returning false");
      return false;
    }
    // If it doesn't return equal, save the new name string and return true
    else
    {
      Serial.println("Control refresh: they are not equal, saving the new name string");
      device.find(msg.device_uuid_val)->second = msg.employee_id_val;
    }
  }
  Serial.println("Control refresh: Returning true");
  return true;
}

void reconnectToTheBroker()
{
  int numberOfConnectionsTried = 0;
  while (!mqttClient.connected())
  {
    Serial.println("Reconnecting to MQTT Broker..");
    if (mqttClient.connect(CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD))
    {
      Serial.println("MQTT Broker Connected.");
      digitalWrite(MQTT_LED,HIGH);
      //subscribe to topic
      mqttClient.subscribe("/name");
      mqttClient.subscribe("/next-event");
    }
    else
    {
      //MQTT Could not reconnect, wifi/esp32 error
      Serial.print("Connection failed, rc=");
      digitalWrite(MQTT_LED,LOW);
      Serial.print(mqttClient.state());
      numberOfConnectionsTried++;
      if (numberOfConnectionsTried > 5)
      {
        Serial.print("Rebooting the device...");
        ESP.restart();
      }
    }
    delay(500);
  }
}

void connectToWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi is diconnected.");
    delay(500);
  }
  Serial.print("Connected to the WiFi.");
}

void callback(char *topic, byte *payload, unsigned int length)
{

  Serial.print("Callback - ");
  Serial.print("Message:");

  char messageTemp[MAX_STRING_SIZE * NUMBER_OF_STRING];
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    messageTemp[i] = payload[i];
  }
  messageTemp[length] = '\0';
  Serial.print("\n");

  const char *delimeter = ";";

  /* -- Getting MacAddresses from both Device and Message -- */
  strcpy(messageMacAddress, strtok(messageTemp, delimeter));
  String getMAC = WiFi.macAddress();
  strcpy(deviceMacAddress, getMAC.c_str());

  Serial.print("deviceMacAddress: ");
  Serial.println(deviceMacAddress);

  if (strcmp(messageMacAddress, deviceMacAddress) == 0)
  {
    //smart desk
    if (strcmp(topic, "/name") == 0)
    {
      topic_flag = false;
      Serial.println("Detected message at the topic name");
      strcpy(msg.device_uuid_val, strtok(NULL, delimeter));
      strcpy(msg.service_uuid_val, msg.device_uuid_val);
      msg.service_uuid_val[7] = '1';
      strcpy(msg.char_uuid_val, msg.device_uuid_val);
      msg.char_uuid_val[7] = '2';
      strcpy(msg.employee_id_val, strtok(NULL, delimeter));

      Serial.print("Device uuid: ");
      Serial.println(msg.device_uuid_val);
      Serial.print("Service uuid: ");
      Serial.println(msg.service_uuid_val);
      Serial.print("Characteristic uuid: ");
      Serial.println(msg.char_uuid_val);
      Serial.print("Employee ID: ");
      Serial.println(msg.employee_id_val);

      // Transmit the message data to queue.
      transmit_flag = true;
    }
    //room reservation
    else if (strcmp(topic, "/next-event") == 0)
    {
      topic_flag = true;
      Serial.println("Detected message at the topic name");
      strcpy(evt.device_uuid_val, strtok(NULL, delimeter));
      strcpy(evt.service_uuid_val, evt.device_uuid_val);
      evt.service_uuid_val[7] = '1';
      strcpy(evt.char_uuid_val, evt.device_uuid_val);
      evt.char_uuid_val[7] = '2';
      strcpy(evt.event_status_val, strtok(NULL, delimeter));
      strcpy(evt.event_time_val, strtok(NULL, delimeter));

      Serial.print("Device uuid: ");
      Serial.println(evt.device_uuid_val);
      Serial.print("Service uuid: ");
      Serial.println(evt.service_uuid_val);
      Serial.print("Characteristic uuid: ");
      Serial.println(evt.char_uuid_val);
      Serial.print("Event Status: ");
      Serial.println(evt.event_status_val);
      Serial.print("Event Time: ");
      Serial.println(evt.event_time_val);

      // Transmit the message data to queue.
      transmit_flag = true;
    }
  }
  else
  {
    Serial.println("Different device");
  }
}

void setupMQTT()
{
  //wifiClient.setCACert(ca_cert);
  mqttClient.setServer(MQTT_SERVER_NAME, MQTT_PORT);
  // set the callback function
  mqttClient.setCallback(callback);
  mqttClient.setKeepAlive(60);
}

// TODO: Create a mqtt_task
// Connect to broker.
// Take the data package.
// Parse it.
// Add to the queue struct.
// Signal to the BLE Task.
static void mqtt_task(void *argp)
{
  esp_err_t er;

  er = esp_task_wdt_add(nullptr);
  assert(er == ESP_OK);

  BaseType_t rc;
  for (;;)
  {
    esp_task_wdt_reset();
    // Connect to broker
    if (!mqttClient.connected())
    {
      Serial.println("Reconnecting to the broker..");
      reconnectToTheBroker();
    }
    // Take the data package, parse it.
    mqttClient.loop();

    if (transmit_flag == true)
    {
      transmit_flag = false;

      Serial.println("Giving the semaphore..");
      // Signal to ble_task
      rc = xSemaphoreGive(barrier);
      // assert(rc == pdPASS);
    }
  }
}

// TODO: Create a ble task

// Take the queue values.
// Scan the ESL devices.
// Connect to ESL according to the UUID value in the Queue.
// Send the data package. (If the data is bigger than 20 bytes then send it respectively.)
// Disconnect from device.
// Signal to the listener task.
static void ble_task(void *argp)
{

  BaseType_t s;
  BaseType_t rc;

  for (;;)
  {
    rc = xSemaphoreTake(barrier, portMAX_DELAY);
    assert(rc == pdPASS);
    Serial.println("Taking the semaphore..");

    // Transmit to BLEUUID
    if (topic_flag)
    {
      device_uuid = BLEUUID(evt.device_uuid_val);
      service_uuid = BLEUUID(evt.service_uuid_val);
      char_uuid = BLEUUID(evt.char_uuid_val);
    }
    else
    {
      device_uuid = BLEUUID(msg.device_uuid_val);
      service_uuid = BLEUUID(msg.service_uuid_val);
      char_uuid = BLEUUID(msg.char_uuid_val);
    }

    // Scan the BLE devices.
    BLEScan *pBLEScan = BLEDevice::getScan();

    // Connect to ESL according to the UUID value in the Queue.
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(15, false);

    if (doConnect == true)
    {
      if (connectToServer())
      {
        Serial.println("We are now connected to the BLE Server.");
      }
      else
      {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }

      doConnect = false;
    }
    else
    {
      Serial.println("doConnect = false");
      Serial.println(mqttClient.publish("/warning", "{ \"mac\": \"3c:71:bf:f5:5d:58\",\"warningExp\": \"ShelfLabel - UUID not found\", \"warningCode\": 2 }"));
    }

    if (connected)
    {
      String newString = "";

      if (topic_flag)
      {
        char *temp;
        temp = strcat(evt.event_status_val, "^");
        newString = strcat(temp, evt.event_time_val);
        Serial.println("Setting new characteristic value..");
        Serial.println(newString);
      }
      else
      {
        if (sizeof(msg.employee_id_val) > 20)
        {
          char temp[20] = {0};
          strncpy(temp, msg.employee_id_val, 20);
          temp[19] = '.';
          newString = temp;
          newString.replace(" ", "^");
        }
        else
        {
          newString = msg.employee_id_val;
          newString.replace(" ", "^");
        }
        Serial.println("Setting new characteristic value..");
        Serial.println(newString);
      }

      // Set the characteristic's value to be the array of bytes that is actually a string.
      pRemoteCharacteristic->writeValue(newString.c_str(), newString.length());
      pClient->disconnect();
    }
    // Serial.println("Listener task is resuming...");
    // vTaskResume(h_listener);
  }
}

void setup()
{
  pinMode(MQTT_LED,OUTPUT);
  digitalWrite(MQTT_LED,LOW);

  int app_cpu = xPortGetCoreID();
  BaseType_t rc;
  esp_err_t er;

  htask = xTaskGetCurrentTaskHandle();
  loopTaskWDTEnabled = true;

  barrier = xSemaphoreCreateBinary();
  assert(barrier);

  Serial.begin(115200);
  connectToWiFi();
  setupMQTT();
  Serial.println("Starting ESP32 Gateway application...");
  BLEDevice::init("");

  delay(2000); // Allow USB to connect

  er = esp_task_wdt_status(htask);
  assert(er == ESP_ERR_NOT_FOUND);

  if (er == ESP_ERR_NOT_FOUND)
  {
    er = esp_task_wdt_init(5, true);
    assert(er == ESP_OK);
    er = esp_task_wdt_add(htask);
    assert(er == ESP_OK);
  }

  rc = xTaskCreatePinnedToCore(
      mqtt_task,
      "mqtttask",
      10000, // Stack Size
      nullptr,
      1,       // Priortiy
      nullptr, // Task Handle
      app_cpu  // CPU
  );
  assert(rc == pdPASS);
  //assert(h_listener);

  rc = xTaskCreatePinnedToCore(
      ble_task,
      "bletask",
      10000, // Stack Size
      nullptr,
      1,       // Priortiy
      nullptr, // Task Handle
      app_cpu  // CPU
  );
  assert(rc == pdPASS);
}

void loop()
{
  esp_err_t er;
  er = esp_task_wdt_status(htask);
  assert(er == ESP_OK);
}
