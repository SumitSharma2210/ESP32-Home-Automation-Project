#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <Ticker.h>
#include <stdlib_noniso.h>

#define DPIN 4
#define doorPin 19
#define roomPin 18
#define LED_BUILTIN 2

const char* ssid = "Homenet";
const char* password = "Sumit@72";

DHT dht(DPIN, DHT11);
Ticker lightsTicker;
WebServer server(80);

bool doorState = LOW;
float temperature = 0.0;
float humidity = 0.0;
String alarmTimestamp = "0";

void handleCors() {
  if (server.method() == HTTP_OPTIONS) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  }
}

void setup() {
  pinMode(doorPin, OUTPUT);
  pinMode(roomPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Ensure the relay (roomPin) is initially off
  digitalWrite(roomPin, LOW);

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.onNotFound(handleCors);

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"message\": \"Hello from ESP32!\"}");
  });

  server.on("/temp", HTTP_GET, []() {
    handleCors();
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    DynamicJsonDocument doc(100);
    JsonObject root = doc.to<JsonObject>();
    root["temp"] = temperature;
    root["humidity"] = humidity;

    String response;
    serializeJson(doc, response);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
  });

  server.on("/alarm", HTTP_POST, []() {
    if (server.hasArg("delayMillis")) {
      String delayMillisStr = server.arg("delayMillis");
      alarmTimestamp = server.arg("alarmTimestamp");
      long int delayMillis = atol(delayMillisStr.c_str()); 
      scheduleAlarm(delayMillis);
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "application/json", "{\"message\": \"Alarm set\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing delayMillis parameter\"}");
    }
  });

  server.on("/immediateAlarm", HTTP_POST, []() {
    if (server.hasArg("delayMillis")) {
      String delayMillisStr = server.arg("delayMillis");
      alarmTimestamp = server.arg("alarmTimestamp");
      long int delayMillis = atol(delayMillisStr.c_str()); 
      scheduleAlarm(delayMillis);
      //digitalWrite(roomPin, HIGH); // Immediately turn on the relay
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "application/json", "{\"message\": \"Immediate alarm set and relay turned on\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing delayMillis parameter\"}");
    }
  });

  server.on("/cancelAlarm", HTTP_POST, []() {
    alarmTimestamp = "0";
    lightsTicker.detach();
    digitalWrite(roomPin, LOW); // Immediately turn off the relay
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"message\": \"Alarm cancelled and relay turned off\"}");
  });

  // Endpoint to turn on the LED
  server.on("/turnOnLED", HTTP_POST, []() {
    digitalWrite(roomPin, HIGH);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"message\": \"LED turned on\"}");
  });

  // Endpoint to turn off the LED
  server.on("/turnOffLED", HTTP_POST, []() {
    digitalWrite(roomPin, LOW);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"message\": \"LED turned off\"}");
  });

  server.begin();
  dht.begin();
}

void scheduleAlarm(long int delayMillis) {
  Serial.print("Scheduling alarm to run after: ");
  Serial.println(delayMillis);
  if (delayMillis > 0) {
    lightsTicker.once_ms(delayMillis, turnOnLight);
    Serial.println("Alarm scheduled successfully.");
  } else {
    Serial.println("Error: delayMillis is non-positive.");
  }
}

void turnOnLight() {
  Serial.println("Alarm triggered! Turning on the light.");
  digitalWrite(roomPin, HIGH); // Turn on the relay
}

void loop() {
  server.handleClient();

  static unsigned long lastUpdateTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= 2000) {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    lastUpdateTime = currentTime;
  }
}
