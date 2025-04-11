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
float humidity = -1;
float windKph = -1;
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
  } else {
    Serial.println("\nVerbindung fehlgeschlagen!");
    scrollError("No WiFi");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    updateWeatherData();
    scrollWeatherData();
  } else {
    Serial.println("WiFi nicht verbunden, versuche erneut...");
    scrollError("No WiFi");
    WiFi.begin(ssid, password);
  }
}

void updateWeatherData() {
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
      humidity = doc["current"]["humidity"];
      windKph = doc["current"]["wind_kph"];
      
      Serial.print("Temperature: "); Serial.print(temperatureCelsius); Serial.println(" °C");
      Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
      Serial.print("Wind: "); Serial.print(windKph); Serial.println(" km/h");
    } else {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
      scrollError("JSON Err");
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(statusCode);
    scrollError("HTTP Err");
  }
}

void scrollWeatherData() {
  char weatherStr[64];
  char tempStr[6];
  char humStr[6];
  char windStr[6];
  
  dtostrf(temperatureCelsius, 4, 1, tempStr);
  dtostrf(humidity, 4, 1, humStr);
  dtostrf(windKph, 4, 1, windStr);
  
  snprintf(weatherStr, sizeof(weatherStr), " T:%sC H:%s%% W:%skm/h ", 
           tempStr, humStr, windStr);

  while (true) {
    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);
    matrix.textScrollSpeed(100);
    matrix.textFont(Font_4x6);
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(weatherStr);
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
    
    int textLength = strlen(weatherStr) * 4;
    int scrollTime = (textLength * 100) / 10;
    delay(scrollTime);
    
    delay(1000);
    
    if (millis() > 1000) {
      break;
    }
  }
}

void scrollError(const char* errorMsg) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(errorMsg);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}
