#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

const char* ssid = "iPhone von Sandro";
const char* pass = "0987654321";

#define RA8876_CONFIG       RA8876_CONFIG_1280x720
#define RA8876_COLOR_DEPTH  16
#define RA8876_SPI_SPEED    1000000
#define RA8876_CS           10
#define RA8876_RESET        2

#include "ra8876.h"
#include "ra8876-config.h"
#include "ra8876-implementation.h"

RA8876 tft = RA8876(RA8876_CS, RA8876_RESET);

char departuresDir1[3][5][32], departuresDir2[3][5][32];
size_t countDir1 = 0, countDir2 = 0;

bool fetchDepartures(int direction, char out[3][5][32], size_t &outCount) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Skipping request.");
    return false;
  }

  Serial.print("Fetching departures for direction ");
  Serial.println(direction);

  WiFiClient wifi;
  HttpClient client(wifi, "transport.integration.sl.se", 80);

  client.connectionKeepAlive();  

  String url = "/v1/sites/9179/departures?transport=METRO&forecast=180&direction=" + String(direction);
  client.get(url);
  
  int statusCode = client.responseStatusCode();
  Serial.print("HTTP Status Code: ");
  Serial.println(statusCode);
  if (statusCode != 200) {
    Serial.println("Failed to fetch data!");
    client.stop(); 
    return false;
  }

  String body = client.responseBody();
  Serial.println("Received JSON payload:");
  Serial.println(body);

  StaticJsonDocument<512> filter;
  filter["departures"][0]["line"]["designation"] = true;
  filter["departures"][0]["line"]["transport_mode"] = true;
  filter["departures"][0]["destination"] = true;
  filter["departures"][0]["expected"] = true;
  filter["departures"][0]["display"] = true;

  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc, body, DeserializationOption::Filter(filter)) != DeserializationError::Ok) {
    Serial.println("JSON deserialization failed!");
    client.stop();
    return false;
  }

  JsonArray arr = doc["departures"];
  outCount = min(3, arr.size());
  Serial.print("Parsed ");
  Serial.print(outCount);
  Serial.println(" departures.");

  for (size_t i = 0; i < outCount; i++) {
    JsonObject d = arr[i];
    snprintf(out[i][0], 32, "%s", d["line"]["designation"] | "");
    snprintf(out[i][1], 32, "%s", d["line"]["transport_mode"] | "");
    snprintf(out[i][2], 32, "%s", d["destination"] | "");
    snprintf(out[i][3], 32, "%.5s", (const char*)(d["expected"] | "") + 11);
    snprintf(out[i][4], 32, "%s", d["display"] | "");
  }

  client.stop();
  return true;
}


void printUtf8Safe(const char* str) {
  while (*str) {
    if ((uint8_t)str[0] == 0xC3) {
      switch ((uint8_t)str[1]) {
        case 0xA5: tft.putChar(0xE5); break;
        case 0xA4: tft.putChar(0xE4); break;
        case 0xB6: tft.putChar(0xF6); break;
        case 0x85: tft.putChar(0xC5); break;
        case 0x84: tft.putChar(0xC4); break;
        case 0x96: tft.putChar(0xD6); break;
        default:   tft.putChar('?');  break;
      }
      str += 2;
    } else tft.putChar(*str++);
  }
}

void showDepartures() {
  Serial.println("Updating display with new departure data...");
  tft.clearScreen(Color::Black);
  tft.setFont(RA8876_FONT_SIZE_32, RA8876_FONT_ENCODING_8859_1);
  tft.setTextScale(1);
  uint16_t cols[] = { 50, 150, 300, 600, 700 }, y = 80;

  tft.setTextColor(Color::White);
  tft.setTextCursor(50, y - 40); tft.print("Riktning "); printUtf8Safe(departuresDir2[0][2]);
  tft.setTextCursor(cols[0], y); tft.print("LINE");
  tft.setTextCursor(cols[1], y); tft.print("TYPE");
  tft.setTextCursor(cols[2], y); tft.print("DESTINATION");
  tft.setTextCursor(cols[3], y); tft.print("TIME");
  tft.setTextCursor(cols[4], y); tft.print("ETA");

  y += 40;
  for (size_t i = 0; i < countDir2; i++) {
    tft.setTextColor(Color::Yellow); tft.setTextCursor(cols[0], y); tft.print(departuresDir2[i][0]);
    tft.setTextColor(Color::White);
    tft.setTextCursor(cols[1], y); tft.print(departuresDir2[i][1]);
    tft.setTextCursor(cols[2], y); printUtf8Safe(departuresDir2[i][2]);
    tft.setTextCursor(cols[3], y); tft.print(departuresDir2[i][3]);
    tft.setTextCursor(cols[4], y); tft.print(departuresDir2[i][4]);
    y += 40;
  }

  y += 40;
  tft.setTextCursor(50, y - 40); tft.print("Riktning "); printUtf8Safe(departuresDir1[0][2]);
  tft.setTextCursor(cols[0], y); tft.print("LINE");
  tft.setTextCursor(cols[1], y); tft.print("TYPE");
  tft.setTextCursor(cols[2], y); tft.print("DESTINATION");
  tft.setTextCursor(cols[3], y); tft.print("TIME");
  tft.setTextCursor(cols[4], y); tft.print("ETA");

  y += 40;
  for (size_t i = 0; i < countDir1; i++) {
    tft.setTextColor(Color::Yellow); tft.setTextCursor(cols[0], y); tft.print(departuresDir1[i][0]);
    tft.setTextColor(Color::White);
    tft.setTextCursor(cols[1], y); tft.print(departuresDir1[i][1]);
    tft.setTextCursor(cols[2], y); printUtf8Safe(departuresDir1[i][2]);
    tft.setTextCursor(cols[3], y); tft.print(departuresDir1[i][3]);
    tft.setTextCursor(cols[4], y); tft.print(departuresDir1[i][4]);
    y += 40;
  }
}

unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!tft.init()) {
    Serial.println("TFT init failed!");
    while (1) delay(100);
  }
  Serial.println("TFT init success.");

  tft.setFont(RA8876_FONT_SIZE_32, RA8876_FONT_ENCODING_8859_1);
  tft.clearScreen(Color::Black);

  fetchDepartures(2, departuresDir2, countDir2);
  fetchDepartures(1, departuresDir1, countDir1);
  showDepartures();
  lastUpdate = millis();
}

void loop() {
  if (millis() - lastUpdate > 30000) {
    Serial.println("Refreshing departure data...");
    if (fetchDepartures(2, departuresDir2, countDir2) &&
        fetchDepartures(1, departuresDir1, countDir1)) {
      showDepartures();
    }
    lastUpdate = millis();
  }
}
