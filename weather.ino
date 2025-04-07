#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>

const char* ssid = "iPhone von Sandro"; 
const char* password = "0987654321"; 
const char* apiKey = "e5c739b8d48e4398a54102549250704";
const char* latitude = "59.254369155424335";
const char* longitude = "18.082729726888548";
const char* server = "api.weatherapi.com";

float temperatureCelsius = -100;
WiFiClient client;
ArduinoLEDMatrix matrix;

void setup() {
  Serial.begin(115200);
  matrix.begin();
  
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int timeout = 60;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi verbunden!");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nVerbindung fehlgeschlagen!");
    showError("No WiFi");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    updateTemperature();
    displayTemperature();
  } else {
    Serial.println("WiFi nicht verbunden, versuche erneut...");
    showError("No WiFi");
    WiFi.begin(ssid, password);
  }
  delay(60000);
}

void updateTemperature() {
  String path = "/v1/current.json?key=";
  path += apiKey;
  path += "&q=";
  path += latitude;
  path += ",";
  path += longitude;
  
  HttpClient http(client, server, 80);
  http.get(path);
  
  int statusCode = http.responseStatusCode();
  String responseBody = http.responseBody();
  
  if (statusCode == 200) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, responseBody);
    
    if (!error) {
      temperatureCelsius = doc["current"]["temp_c"];
      Serial.print("Temperature: ");
      Serial.print(temperatureCelsius);
      Serial.println(" °C");
    } else {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
      showError("JSON Err");
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(statusCode);
    showError("HTTP Err");
  }
}

void displayTemperature() {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  
  char tempStr[8];
  dtostrf(temperatureCelsius, 5, 1, tempStr);
  
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(tempStr);
  matrix.print("C");
  matrix.endText(NO_SCROLL);
  
  matrix.endDraw();
}

void showError(const char* errorMsg) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(errorMsg);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}
