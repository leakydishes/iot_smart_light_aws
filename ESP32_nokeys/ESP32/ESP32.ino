#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h> 
#include <ArduinoJson.h> 
#include "WiFi.h"

// constants
const int PIR_SENSOR_OUTPUT_PIN = 13;  // PIR sensor O/P pin
const int LIGHT_SENSOR_PIN = 36;       // LDR sensor pin
const int LED_PIN = 22;                // LED pin
const int ANALOG_THRESHOLD = 500;      // LDR threshold value
const int DAYLIGHT_THRESHOLD = 600;    // Adjust as needed for daylight detection
const int LIGHT_ON_DURATION = 30000;   // 30 seconds in milliseconds
const int CHECK_INTERVAL = 10000;      // 10 seconds in milliseconds
const int CONTACT_IT_SUPPORT_INTERVAL = 60000;  // 60 seconds in milliseconds

enum LightStatus { LIGHT_OFF, LIGHT_ON };

LightStatus lightStatus = LIGHT_OFF;
unsigned long lightTurnedOnTime = 0;
unsigned long lastCheckTime = 0;
unsigned long lastContactSupportTime = 0;

// MQTT topics for the device
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure wifi_client = WiFiClientSecure();
MQTTClient mqtt_client = MQTTClient(256); //256 Max size packets published/ received

uint32_t t1;

void connectAWS()
{
  //Begin WiFi in station mode
  WiFi.mode(WIFI_STA); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  //Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // Configure wifi_client with the correct certificates and keys
  wifi_client.setCACert(AWS_CERT_CA);
  wifi_client.setCertificate(AWS_CERT_CRT);
  wifi_client.setPrivateKey(AWS_CERT_PRIVATE);

  //Connect to AWS IOT Broker 8883 (port MQTT)
  mqtt_client.begin(AWS_IOT_ENDPOINT, 8883, wifi_client);

  //Set action to be taken on incoming messages
  mqtt_client.onMessage(incomingMessageHandler);

  Serial.print("Connecting to AWS IOT");

  //Wait for connection to AWS IoT
  while (!mqtt_client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  if(!mqtt_client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  //Subscribe to a topic
  mqtt_client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  // Check inputs on digital pins
  int analogValueLDR = analogRead(LIGHT_SENSOR_PIN);
  int analogValuePIR = digitalRead(PIR_SENSOR_OUTPUT_PIN);

  //Create a JSON document of size 200 bytes to populate
  StaticJsonDocument<200> doc;
  doc["elapsed_time"] = millis() - t1;
  doc["lightLDR: "] = analogValueLDR;
   doc["MotionPIR: "] = analogValuePIR;
  doc["deviceName: "] = THINGNAME;
  doc["deviceTime: "] = millis();
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to mqtt_client

  // Publish to the topic
  mqtt_client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  // Print to Serial
  Serial.println("Sent a message");
  Serial.println("Light Sensor Value: ");
  Serial.println(analogValueLDR);
  Serial.println("Motion Sensor Value: ");
  Serial.println(analogValuePIR);
}

void incomingMessageHandler(String &topic, String &payload) {
  Serial.println("Message received!");
  Serial.println("Topic: " + topic);
  Serial.println("Payload: " + payload);
}

// Handle lights and motion
void handleLightControl() {
  unsigned long currentTime = millis();
  int pirSensorOutput = digitalRead(PIR_SENSOR_OUTPUT_PIN);
  int analogValue = analogRead(LIGHT_SENSOR_PIN);

  if (currentTime - lastCheckTime >= CHECK_INTERVAL) {
    lastCheckTime = currentTime;

    if ((pirSensorOutput == HIGH || analogValue < ANALOG_THRESHOLD) && lightStatus == LIGHT_OFF) {
      // Valid sensor readings
      Serial.print("Turning on the light\n\n");
      lightStatus = LIGHT_ON;
      digitalWrite(LED_PIN, HIGH); // Turn on the LED
      lightTurnedOnTime = millis(); // Record the time when the light turned on
    } else {
      // No motion and sufficient light
      if (lightStatus == LIGHT_ON) {
        if (millis() - lightTurnedOnTime >= LIGHT_ON_DURATION) {
          Serial.print("Turning off the light\n\n");
          lightStatus = LIGHT_OFF;
          digitalWrite(LED_PIN, LOW); // Turn off the LED
        }
      }
    }
    // Print sensor values for debugging
    Serial.print("PIR Value: ");
    Serial.println(pirSensorOutput);
    Serial.print("LDR Value: ");
    Serial.println(analogValue);
  }
}

void setup() {
  Serial.begin(115200);
  t1 = millis();
  connectAWS();

  // Initialise Pins
  pinMode(PIR_SENSOR_OUTPUT_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Let ESP32 warm up
  Serial.println("Ready!");
}

void loop() {
  publishMessage();
  mqtt_client.loop();
  handleLightControl();
  delay(4000);
}