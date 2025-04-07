#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "iPhone von Sandro"; 
const char* password = "0987654321"; 
const char* server = "transport.integration.sl.se";
const int siteId = 9181;

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200);
ArduinoLEDMatrix matrix;
int minutesToNext = -1;

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
    timeClient.begin();
    timeClient.update();
    Serial.print("Aktuelle Zeit (UTC+2): ");
    Serial.println(timeClient.getFormattedTime());
  } else {
    Serial.println("\nVerbindung fehlgeschlagen!");
    scrollText("No WiFi");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    updateNextDeparture();
    scrollNextDeparture();
  } else {
    Serial.println("WiFi nicht verbunden, versuche erneut...");
    scrollText("No WiFi");
    WiFi.begin(ssid, password);
  }
}

void updateNextDeparture() {
  String path = "/v1/sites/";
  path += String(siteId);
  path += "/departures";
  
  Serial.print("Sende API-Anfrage an: ");
  Serial.println(path);
  
  HttpClient http(wifiClient, server, 80);
  http.beginRequest();
  http.get(path);
  http.sendHeader("Accept", "application/json");
  http.endRequest();
  
  int statusCode = http.responseStatusCode();
  Serial.print("HTTP-Statuscode: ");
  Serial.println(statusCode);
  
  if (statusCode == 200) {
    String responseBody = http.responseBody();
    Serial.println("API-Antwort erhalten:");
    Serial.println("--- Anfang der Antwort ---");
    Serial.println(responseBody);
    Serial.println("--- Ende der Antwort ---");
    
    if (responseBody.length() == 0) {
      Serial.println("Fehler: Antwort ist leer!");
      scrollText("Empty Resp");
      return;
    }
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, responseBody);
    
    if (!error) {
      JsonArray departures = doc["departures"];
      Serial.print("Anzahl gefundener Abfahrten: ");
      Serial.println(departures.size());
      
      minutesToNext = -1;
      unsigned long currentTime = timeClient.getEpochTime();
      Serial.print("Aktuelle Zeit (Epoch): ");
      Serial.println(currentTime);
      
      for (JsonObject departure : departures) {
        const char* transportMode = departure["line"]["transport_mode"];
        const char* expected = departure["expected"];
        const char* destination = departure["destination"];
        
        Serial.print("Abfahrt - Transport: ");
        Serial.print(transportMode);
        Serial.print(", Ziel: ");
        Serial.print(destination);
        Serial.print(", Erwartet: ");
        Serial.println(expected);
        
        if (strcmp(transportMode, "METRO") == 0) {
          int minutes = calculateMinutes(expected, currentTime);
          Serial.print("Metro gefunden, Minuten bis Abfahrt: ");
          Serial.println(minutes);
          
          if (minutes >= 0 && (minutesToNext == -1 || minutes < minutesToNext)) {
            minutesToNext = minutes;
          }
        }
      }
      
      if (minutesToNext == -1) {
        Serial.println("Keine Metro-Abfahrt gefunden!");
      } else {
        Serial.print("Nächste Metro in: ");
        Serial.print(minutesToNext);
        Serial.println(" Minuten");
      }
    } else {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
      scrollText("JSON Err");
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(statusCode);
    Serial.println("Response: ");
    Serial.println(http.responseBody());
    scrollText("HTTP Err");
  }
}

int calculateMinutes(const char* expected, unsigned long currentTime) {
  int year, month, day, hour, minute, second;
  sscanf(expected, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
  
  struct tm timeinfo = {0};
  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month - 1;
  timeinfo.tm_mday = day;
  timeinfo.tm_hour = hour;
  timeinfo.tm_min = minute;
  timeinfo.tm_sec = second;
  time_t departureTime = mktime(&timeinfo);
  
  int diffSeconds = departureTime - currentTime;
  int minutes = diffSeconds / 60;
  
  Serial.print("Berechnung - Aktuelle Zeit (s): ");
  Serial.print(currentTime);
  Serial.print(", Abfahrtszeit (s): ");
  Serial.print(departureTime);
  Serial.print(", Differenz (min): ");
  Serial.println(minutes);
  
  return minutes;
}

void scrollNextDeparture() {
  char displayStr[32];
  if (minutesToNext >= 0) {
    snprintf(displayStr, sizeof(displayStr), " Next Metro: %d min ", minutesToNext);
  } else {
    snprintf(displayStr, sizeof(displayStr), " No Metro found ");
  }
  
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(displayStr);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
  
  int textLength = strlen(displayStr) * 4;
  int scrollTime = (textLength * 100) / 10;
  delay(scrollTime);
}

void scrollText(const char* text) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}