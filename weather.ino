#include <Arduino.h>
#include <WiFiS3.h>          
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

const char* ssid = "iPhone von Sandro"; 
const char* password = "0987654321"; 

// const char* ssid = "RIoT-5G"; 
// const char* password = "ardiriaj"; 

const char* apiKey = "e5c739b8d48e4398a54102549250704";
const char* latitude = "59.254369155424335";
const char* longitude = "18.082729726888548";
const char* server = "api.weatherapi.com";

float temperatureCelsius = -100;
WiFiSSLClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Arduino Wetterdaten-Client");
  
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
    Serial.println();
    Serial.println("WiFi verbunden!");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Verbindung fehlgeschlagen! Bitte überprüfen Sie SSID und Passwort.");
    Serial.print("WLAN-Status: ");
    Serial.println(WiFi.status());
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Starte API-Anfrage...");
    
    String path = "/v1/current.json?key=";
    path += apiKey;
    path += "&q=";
    path += latitude;
    path += ",";
    path += longitude;
    
    Serial.print("Pfad: ");
    Serial.println(path);
    
    HttpClient http(client, server, 443);
    
    Serial.println("Sende HTTP-Anfrage...");
    http.get(path);
    
    int statusCode = http.responseStatusCode();
    Serial.print("HTTP-Statuscode: ");
    Serial.println(statusCode);
    
    if (statusCode == 200) {
      String payload = http.responseBody();
      Serial.println("Empfangene Daten:");
      Serial.println(payload);
      
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        JsonObject current = doc["current"];
        temperatureCelsius = current["temp_c"];
        
        Serial.print("Temperature: ");
        Serial.print(temperatureCelsius);
        Serial.println(" °C");
      } else {
        Serial.print("JSON parsing error: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(statusCode);
    }
  } else {
    Serial.println("WiFi nicht verbunden, versuche erneut...");
    WiFi.begin(ssid, password);
    delay(5000);
  }
  
  Serial.println("Warte 60 Sekunden bis zur nächsten Messung...");
  delay(60000);
}