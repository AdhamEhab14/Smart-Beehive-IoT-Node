#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include "secrets.h"

// WiFi and Firebase Credintials
const char *WIFI_SSID = SECRET_WIFI_SSID;
const char *WIFI_PASSWORD = SECRET_WIFI_PASS;
const char *FIREBASE_HOST = SECRET_FIREBASE_HOST;
const char *FIREBASE_AUTH = SECRET_FIREBASE_AUTH;

// Sensor Pins
#define DHTPIN 4
#define DHTTYPE DHT11
#define RELAY_PIN 16
#define LED 12

DHT dht(DHTPIN, DHTTYPE);
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// Firebase objects
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

void setup()
{
  Serial.begin(115200);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  // Send Temp every 5 seconds
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000)
  {
    lastSend = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity(); // READ HUMIDITY

    if (!isnan(t) && !isnan(h))
    {
      Firebase.setFloat(firebaseData, "/hive/temperature", t);
      Firebase.setFloat(firebaseData, "/hive/humidity", h); // SEND HUMIDITY
    }
  }

  // listen for the mobile app switch
  if (Firebase.getBool(firebaseData, "/hive/manual_fan"))
  {
    bool remoteSwitch = firebaseData.boolData();
    if (remoteSwitch == true)
    {
      digitalWrite(RELAY_PIN, RELAY_ON);
      digitalWrite(LED, HIGH);
    }
    else
    {
      digitalWrite(RELAY_PIN, RELAY_OFF);
      digitalWrite(LED, LOW);
    }
  }
}