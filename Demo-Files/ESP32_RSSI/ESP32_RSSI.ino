// TODO
/*  What is the code's story?
    1- We should scan the specified BLE devices and get their RSSI values and save them.
    2- We should filter the values in order to increase the accuracy.
    3- We should publish the values to the MQTT Broker's topic.
 */

//For WiFi
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "MQTT_Config.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <math.h>

#define MQTT_CHG 0b0010

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))
#define TX_POWER -59
#define N 3
#define DATA_SIZE 5
#define SIZE 50
#define DATA_SEND 5000 //Per miliseconds
#define LOCAL_MQTT_MAX_PACKET_SIZE 1000

//static SemaphoreHandle_t barrier;
static SemaphoreHandle_t mutex;
static EventGroupHandle_t hevt;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int scanTime = 1; //In seconds
static BLEAddress *pMAC_Address;
int data[DATA_SIZE] = {0};
uint8_t dataIndex = 0;
int temp = 0;
double filtered_rssi = 0;
double filtered_meter = 0;
double raw_meter = 0;
double raw_rssi = 0;
char device_address[17] = {0};
uint8_t message_char_buffer[LOCAL_MQTT_MAX_PACKET_SIZE];


void reconnectToTheBroker()
{
  int numberOfConnectionsTried = 0;
  while (!mqttClient.connected())
  {
    Serial.println("Reconnecting to MQTT Broker..");
    if (mqttClient.connect(CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD))
    {
      Serial.println("MQTT Broker Connected.");
      //subscribe to topic
      mqttClient.subscribe("/name");
      mqttClient.subscribe("/next-event");
    }
    else
    {
      //MQTT Could not reconnect, wifi/esp32 error
      Serial.print("Connection failed, rc=");
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
}

void publishScanDataToMQTT(int filtered_meter, char *device_address)
{

  Serial.print("Publishing the data...");
  String payloadString = "{\"e\":[";

  payloadString += "{\"m\":\"";
  payloadString += String(device_address);
  payloadString += "\",\"r\":";
  payloadString += String(filtered_meter);
  payloadString += "}";

  // SenML ends. Add this stations MAC
  payloadString += "],\"mac\":\"";
  payloadString += String(WiFi.macAddress());
  payloadString += "\"}";

  // Print and publish payload
  Serial.print("MAX len: ");
  Serial.println(LOCAL_MQTT_MAX_PACKET_SIZE);

  Serial.print("Payload length: ");
  Serial.println(payloadString.length());
  Serial.println(payloadString);

  uint8_t messageCharBuffer[LOCAL_MQTT_MAX_PACKET_SIZE];
  payloadString.getBytes(messageCharBuffer, payloadString.length() + 1);

  payloadString.getBytes(message_char_buffer, payloadString.length() + 1);
  int result = mqttClient.publish("/scn-dvc", message_char_buffer, payloadString.length(), false);
  Serial.print("PUB Result: ");
  Serial.println(result);
  // Serial.println(mqttClient.publish("/o1/m1/esp32-1/info/yusuf", "test"));
}

void publishDeviceInfoToMQTT()
{
  //Not implemented. Temp and humidity.
  //result = client.publish("/o1/m1/esp32-1/info", message_char_buffer, payloadString.length(), false);
  //  Serial.print("PUB Result: ");
  //  Serial.println(result);
}

void setupMQTT()
{
  //wifiClient.setCACert(ca_cert);
  mqttClient.setServer(MQTT_SERVER_NAME, MQTT_PORT);
  // set the callback function
  //mqttClient.setCallback(callback);
  mqttClient.setKeepAlive(60);
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.haveServiceUUID() || advertisedDevice.haveName())
    {
      BaseType_t rc;

      if (dataIndex == DATA_SIZE)
      {
        dataIndex = 0;
      }

      data[dataIndex] = advertisedDevice.getRSSI();

      if (dataIndex == DATA_SIZE - 1)
      {
        temp = data[dataIndex];
      }

      //        SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);
      raw_rssi = data[dataIndex];

      if (dataIndex - 1 != -1)
      {
        filtered_rssi = (0.75) * data[dataIndex] + (0.25) * data[dataIndex - 1];
      }
      else
      {
        filtered_rssi = (0.75) * data[dataIndex] + (0.25) * temp;
      }

      raw_meter = pow(10, (double(TX_POWER - raw_rssi) / (10 * N)));

      filtered_meter = pow(10, (double(TX_POWER - filtered_rssi) / (10 * N)));

      Serial.print("MAC Adress: ");
      pMAC_Address = new BLEAddress(advertisedDevice.getAddress());

      strcpy(device_address, advertisedDevice.getAddress().toString().c_str());

      Serial.println(pMAC_Address->toString().c_str());
      Serial.print("Raw rssi: ");
      Serial.println(raw_rssi);
      Serial.print("filtered rssi: ");
      Serial.println(filtered_rssi);

      Serial.print("Raw Meter: ");
      Serial.println(raw_meter);
      Serial.print("Filtered Meter: ");
      Serial.println(filtered_meter);

      if (dataIndex - 1 != -1)
      {
        Serial.print("Previous Data Index: ");
        Serial.println(dataIndex - 1);
        Serial.print("previous Data: ");
        Serial.println(data[dataIndex - 1]);
      }
      else
      {

        Serial.print("Previous Data Index: ");
        Serial.println(DATA_SIZE - 1);
        Serial.print("previous Data: ");
        Serial.println(temp);
      }

      Serial.print("Current Data Index: ");
      Serial.println(dataIndex);
      Serial.print("Current Data: ");
      Serial.println(data[dataIndex]);
      dataIndex++;

      // Give the semaphore
      //Serial.println("Giving the semaphore to the MQTT Task...");
      //rc = xSemaphoreGive(barrier);
      //assert(rc == pdPASS);

      xEventGroupSetBits(hevt, MQTT_CHG);
    } 
  }
};

void lock_task()
{
  BaseType_t rc;

  rc = xSemaphoreTake(mutex, portMAX_DELAY);
  assert(rc == pdPASS);
}

void unlock_task()
{
  BaseType_t rc;

  rc = xSemaphoreGive(mutex);
  assert(rc == pdPASS);
}

/* TODO

- Surekli scanning yapmali ve MAC adresi kayitli olan beaconlarin RSSI degerlerini kaydetmesi lazim.

- Kaydettigi RSSI degerleri belirli bir sayiya ulastiginda yani salliyorum 20 kez kaydetti,
  o zaman filter task ine bi sinyal cakacak ve queue yu gonderecek.

- Ama burada soyle bir olay var. Kac tane device bulduysa ona gore filter_task ini cogaltmasi gerek sanirim.
  O filter taskleri de calisip senkronize bir sekilde mqtt task ine datalari gondermesi lazim.
*/
static void ble_task(void *argp)
{
  // SETUP BLE
  
  lock_task();
  Serial.println("Mutex locked in ble task.");
  Serial.println("Scanning...");
  BLEDevice::init("");
  BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
  Serial.println("Mutex unlocked in ble task.");
  unlock_task();
  
  for (;;)
  {
    lock_task();
    Serial.println("Mutex locked in ble task.");
    // SCANNER LOOP
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.print("Devices found: ");
    Serial.println(foundDevices.getCount());
    Serial.println("Scan done!");

    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
    delete pMAC_Address;
    pMAC_Address = NULL;
    Serial.println("Mutex unlocked in ble task.");
    unlock_task();
    delay(500);

    // TODO
    // low pass filter needed
    // When the filtering is finished, signal to mqtt_task
  }
}

/* TODO

- MQTT broker ina connection istegi gonderecek.

- Connect olduktan sonra filter_task datasini topic'e publish edecek.
*/
static void mqtt_task(void *argp)
{
  BaseType_t rc;

  for (;;)
  {
    //rc = xSemaphoreTake(barrier, portMAX_DELAY);
    //assert(rc == pdPASS);
    //Serial.println("Taking the semaphore..");

    xEventGroupWaitBits(
      hevt,            // Event group
      MQTT_CHG,        // bits to wait for
      pdTRUE,         // clear the bits
      pdFALSE,         // wait for all bits
      portMAX_DELAY);  // timeout

    lock_task();
    Serial.println("Mutex locked in mqtt task.");
    // Connect to broker
    if (!mqttClient.connected())
    {
      Serial.println("Reconnecting to the broker..");
      reconnectToTheBroker();
    }
    publishScanDataToMQTT(filtered_meter, device_address);
    publishDeviceInfoToMQTT();
    Serial.println("Mutex unlocked in mqtt task.");
    unlock_task();
  }
}
void setup()
{
  Serial.begin(115200);

  int app_cpu = xPortGetCoreID();
  BaseType_t rc;

  mutex = xSemaphoreCreateMutex();
  assert(mutex);

  //barrier = xSemaphoreCreateBinary();
  //assert(barrier);

  hevt = xEventGroupCreate();
  assert(hevt);

  Serial.println("Starting ESP32 RSSI filter application...");

  delay(2000); // Allow USB to connect

  rc = xTaskCreatePinnedToCore(
      mqtt_task,
      "mqtttask",
      5000, // Stack Size
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
      5000, // Stack Size
      nullptr,
      1,       // Priortiy
      nullptr, // Task Handle
      app_cpu  // CPU
  );
  assert(rc == pdPASS);

  connectToWiFi();
  setupMQTT();
  BLEDevice::init("");
}

void loop()
{
  // Delete loop task.
  vTaskDelete(nullptr);
}
