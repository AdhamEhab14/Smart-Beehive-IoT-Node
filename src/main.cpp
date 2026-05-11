#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define DHTPIN 15
#define RELAY_PIN 2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Credentials
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// HiveMQ Credentials
const char *mqtt_server = "1eaa321f27b84b728ab7a3fac9efd010.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_user = "Adham14";
const char *mqtt_pass = "Adham102030";

// global objects
WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHT22);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RTOS Handlers
TaskHandle_t SensorTask;
TaskHandle_t MqttTask;

void setup_wifi();
void reconnect();
void callback(char *topic, byte *payload, unsigned int length);
void sensorTaskCode(void *parameter);
void mqttTaskCode(void *parameter);
void updateOLED(float t, float h);

void setup()
{
  Serial.begin(115200);
  delay(1000);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  dht.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Booting System...");
  display.display();

  // Configure secure WiFi for HiveMQ Cloud
  espClient.setInsecure(); // Skips certificate validation for prototyping

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // MQTT connection and listening for commands Task
  xTaskCreatePinnedToCore(
      mqttTaskCode,
      "MQTT_Task",
      10000, // Stack size
      NULL,
      1, // Priority
      &MqttTask,
      0); // Core 0

  // Sensor Task
  xTaskCreatePinnedToCore(
      sensorTaskCode,
      "Sensor_Task",
      10000,
      NULL,
      1,
      &SensorTask,
      1);
}

void loop()
{
  vTaskDelete(NULL);
}

// continuously read sensors and publish data
void sensorTaskCode(void *parameter)
{
  int loopCounter = 0; // Each increment is 500ms

  for (;;)
  {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t))
    {
      // 1. UPDATE OLED IMMEDIATELY
      updateOLED(t, h);

      if (loopCounter >= 10)
      {
        if (client.connected())
        {
          String payload = "{\"temperature\": " + String(t) + ", \"humidity\": " + String(h) + "}";
          Serial.print("Cloud Update: ");
          Serial.println(payload);
          client.publish("beehive/telemetry", payload.c_str());
        }
        loopCounter = 0;
      }
    }

    loopCounter++;
    // Run the loop every 500ms
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// keep MQTT alive and process incoming commands
void mqttTaskCode(void *parameter)
{
  for (;;)
  {
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();                       // allows the library to receive incoming messages
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay
  }
}

void setup_wifi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-Wokwi-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
    {
      Serial.println("connected to HiveMQ!");
      // Subscribe to the command topic
      client.subscribe("beehive/commands");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

// This function runs whenever a message arrives from the cloud
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  // If the cloud sends "HEAT_ON", turn on the relay
  if (String(topic) == "beehive/commands")
  {
    if (messageTemp == "HEAT_ON")
    {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("-> Action: Relay turned ON");
    }
    else if (messageTemp == "HEAT_OFF")
    {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("-> Action: Relay turned OFF");
    }
  }
}

void updateOLED(float t, float h)
{
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("WiFi: ");
  display.println(WiFi.status() == WL_CONNECTED ? "OK" : "ERR");

  // Temperature
  display.setCursor(0, 16);
  display.setTextSize(2);
  display.print(t, 1);
  display.print(" C");

  // Humidity
  display.setCursor(0, 36);
  display.print(h, 1);
  display.print(" %");

  // Fan/Relay Status
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("Fan Status: ");
  display.print(digitalRead(RELAY_PIN) == HIGH ? "ON" : "OFF");

  display.display();
}