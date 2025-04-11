//--------------------------------------------------------------------------
// Copyright 2024, RIoT Secure AB
//
// @author Aaron Ardiri
//--------------------------------------------------------------------------

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

//--------------------------------------------------------------------------
// CONFIGURATION
//--------------------------------------------------------------------------

#define ENABLE_DEBUG

#define RA8876_CONFIG_800x600   0  // 4:3
#define RA8876_CONFIG_1024x600  1
#define RA8876_CONFIG_1024x768  2  // 4:3
#define RA8876_CONFIG_1280x720  3  // 16:9 720p
#define RA8876_CONFIG_1366x768  4  // 16:9

#define RA8876_CONFIG           RA8876_CONFIG_1280x720
#define RA8876_COLOR_DEPTH      16 // 8, 16 or 24 (depth of video output)

#if defined(ENABLE_DEBUG)
#define dbgUART Serial
#endif

//--------------------------------------------------------------------------
// PINOUT
//--------------------------------------------------------------------------

#define RA8876_SCK       13
#define RA8876_MOSI      11
#define RA8876_MISO      12  // standard SPI pins
#define RA8876_CS        10

#define RA8876_SPI_SPEED 1000000

#define RA8876_INTERRUPT 3
#define RA8876_RESET     2

//--------------------------------------------------------------------------
// RA8876 :: SUPPORT
//--------------------------------------------------------------------------

#include "ra8876.h"
#include "ra8876-config.h"
#include "ra8876-implementation.h"

RA8876 tft = RA8876(RA8876_CS, RA8876_RESET);

//--------------------------------------------------------------------------
// APPLICATION
//--------------------------------------------------------------------------

#define PROGMEM_FAR

const char* ssid = "iPhone von Sandro"; 
const char* password = "0987654321"; 
const char* server = "transport.integration.sl.se";
const int siteId = 9183;

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200);
int minutesToNext = -1;

void
setup()
{
#if defined(ENABLE_DEBUG)
  dbgUART.begin(115200);
//while (!dbgUART) ; 
#endif

  // wait a while for USB to connect
  delay(2500);

  // initialize the RA8876
#if defined(ENABLE_DEBUG)
  dbgUART.print("* initializing RA8876: ");
  if (!tft.init())
  {
    dbgUART.println("failed");
    dbgUART.println(":: failed to intiialzie RA8876"); while (1) delay(50); 
  }
  dbgUART.println("done");
#else
  tft.init();
#endif

  // lets show the color bar test while we initialize
  tft.colorBarTest(true);

  // clear the screen
  tft.clearScreen(Color::Black);

  // ok - we are done; we can turn off the color bar test and do our demo!
  tft.colorBarTest(false);
  
  WiFi.begin(ssid, password);
  
  int timeout = 60;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    timeout--;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.begin();
    timeClient.update();
  } else {
    dbgUART.print("No WiFi");
  }
}

void transitDisplay() {
  uint16_t width;
  uint16_t height;
  uint16_t y;
  
#if defined(ENABLE_DEBUG)
  dbgUART.println(F("* transitDisplay() "));
#endif

  // display properties
  width = tft.getDisplayWidth();
  height = tft.getDisplayHeight();

#if defined(ENABLE_DEBUG)
  dbgUART.print(F("LCD width  = "));
  dbgUART.println(width);
  dbgUART.print(F("LCD height = "));
  dbgUART.println(height);
#endif

  // Fetch latest transit data
  updateTransitData();

  // clear the screen
  tft.clearScreen(Color::Black);

  // Draw header and icon (reusing your plane icon as a transit icon)
  {
    uint8_t plane[][2] = { 
      { 50 +  6, 50 + 29 }, { 50 + 14, 50 + 34 }, { 50 + 42, 50 + 22 },
      { 50 + 43, 50 + 18 }, { 50 + 39, 50 + 18 }, { 50 + 14, 50 + 28 }, 
      { 50 +  9, 50 + 26 }, { 50 + 20, 50 + 25 }, { 50 + 30, 50 + 21 },
      { 50 + 12, 50 + 20 }, { 50 + 12, 50 + 18 } 
    };
    Color color = Color::White;

    tft.drawRectangle(50, 50, 100, 100, color);
    tft.drawRectangle(51, 51, 99, 99, color);

    // Draw plane icon (could be replaced with a bus/metro icon later)
    tft.fillTriangle(plane[ 0][0], plane[ 0][1], plane[ 6][0], plane[ 6][1], plane[ 1][0], plane[ 1][1], color);
    tft.fillTriangle(plane[ 6][0], plane[ 6][1], plane[ 1][0], plane[ 1][1], plane[ 5][0], plane[ 5][1], color);
    tft.fillTriangle(plane[ 1][0], plane[ 1][1], plane[ 5][0], plane[ 5][1], plane[ 2][0], plane[ 2][1], color);
    tft.fillTriangle(plane[ 5][0], plane[ 5][1], plane[ 2][0], plane[ 2][1], plane[ 4][0], plane[ 4][1], color);
    tft.fillTriangle(plane[ 4][0], plane[ 4][1], plane[ 2][0], plane[ 2][1], plane[ 3][0], plane[ 3][1], color);
    tft.fillTriangle(plane[ 7][0], plane[ 7][1], plane[ 8][0], plane[ 8][1], plane[10][0], plane[10][1], color);
    tft.fillTriangle(plane[ 7][0], plane[ 7][1], plane[ 8][0], plane[ 8][1], plane[ 9][0], plane[ 9][1], color);

    tft.fillRectangle(57, 93, 92, 95, color);
  }

  // draw the title row
  tft.setFont(RA8876_FONT_SIZE_24);

  tft.setTextScale(2);  
  tft.setTextColor(Color::White);
  tft.setTextCursor(125, 48);
  tft.print(F("DEPARTURES - HÖKARÄNGEN"));

  // Define new column positions for transit data
  uint16_t cols[] = { 50, 125, 420, 550, 800, 1175 };

  tft.setTextScale(1);
  tft.setTextCursor(cols[0], 150);
  tft.print(F("LINE"));

  tft.setTextCursor(cols[1], tft.getTextCursorY());
  tft.print(F("TYPE"));

  tft.setTextCursor(cols[2], tft.getTextCursorY());
  tft.print(F("DESTINATION"));

  tft.setTextCursor(cols[3], tft.getTextCursorY());
  tft.print(F("DEPARTURE"));

  tft.setTextCursor(cols[4], tft.getTextCursorY());
  tft.print(F("STATUS"));

  tft.drawLine(50, 180, width - 50, 180, Color::White);
  tft.drawLine(50, 181, width - 50, 181, Color::White);

  // Setup for displaying data
  tft.setFont(RA8876_FONT_SIZE_32);
  y = 225;
  
  // Display the current time from NTP client
  String currentTimeStr = timeClient.getFormattedTime();
  tft.setTextColor(Color::Cyan);
  tft.setTextCursor(width - 200, 48);
  tft.print(currentTimeStr);

  // Get and parse transit data
  if (WiFi.status() == WL_CONNECTED) {
    String path = "/v1/sites/";
    path += String(siteId);
    path += "/departures";
    
    HttpClient http(wifiClient, server, 80);
    http.beginRequest();
    http.get(path);
    http.sendHeader("Accept", "application/json");
    http.endRequest();
    
    int statusCode = http.responseStatusCode();
    
    if (statusCode == 200) {
      String responseBody = http.responseBody();
      
      if (responseBody.length() == 0) {
        tft.setTextColor(Color::Red);
        tft.setTextCursor(cols[0], y);
        tft.print("No data received from API");
        return;
      }
      
      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, responseBody);
      
      if (!error) {
        JsonArray departures = doc["departures"];
        int count = 0;
        
        for (JsonObject departure : departures) {
          if (count >= 10) break; // Display max 10 departures to fit screen
          
          const char* designation = departure["line"]["designation"];
          const char* transportMode = departure["line"]["transport_mode"];
          const char* destination = departure["destination"];
          const char* display = departure["display"];  // "2 min" etc
          const char* expected = departure["expected"];
          const char* state = departure["state"];
          
          unsigned long currentTime = timeClient.getEpochTime();
          int minutes = calculateMinutes(expected, currentTime);
          
          // Set colors based on transport mode
          if (strcmp(transportMode, "METRO") == 0) {
            tft.setTextColor(Color::Green);
          } else if (strcmp(transportMode, "BUS") == 0) {
            tft.setTextColor(Color::Yellow);
          } else {
            tft.setTextColor(Color::White);
          }
          
          // Line number
          tft.setTextCursor(cols[0], y);
          tft.print(designation);
          
          // Transport type
          tft.setTextCursor(cols[1], y);
          tft.print(transportMode);
          
          // Destination 
          tft.setTextCursor(cols[2], y);
          tft.print(destination);
          
          // Time until departure
          tft.setTextCursor(cols[3], y);
          if (minutes <= 0) {
            tft.print("Now");
          } else {
            tft.print(String(minutes) + " min");
          }
          
          // Status
          tft.setTextCursor(cols[4], y);
          if (strcmp(state, "EXPECTED") == 0) {
            tft.setTextColor(Color::Green);
            tft.print("ON TIME");
          } else {
            tft.setTextColor(Color::Orange);
            tft.print(state);
          }
          
          y += tft.getTextHeight();
          count++;
        }
        
        if (count == 0) {
          tft.setTextColor(Color::Red);
          tft.setTextCursor(cols[0], y);
          tft.print("No departures found");
        }
      } else {
        tft.setTextColor(Color::Red);
        tft.setTextCursor(cols[0], y);
        tft.print("JSON parsing error");
      }
    } else {
      tft.setTextColor(Color::Red);
      tft.setTextCursor(cols[0], y);
      tft.print("HTTP error: " + String(statusCode));
    }
  } else {
    tft.setTextColor(Color::Red);
    tft.setTextCursor(cols[0], y);
    tft.print("WiFi not connected");
  }

  // Keep display active and update periodically
  uint32_t ts = millis();
  uint32_t ats = ts;
  bool pos = true;

  // Display for 30 seconds, then refresh
  while (millis() - ts < 30000) {
    // Update blinking indicator every second
    if ((millis() - ats) > 1000) {
      ats = millis();
      pos = !pos;
      
      // Show a blinking indicator (clock/update indicator)
      if (pos) {
        tft.fillCircle(width - 50, 60, 10, Color::Yellow);
      } else {
        tft.fillCircle(width - 50, 60, 10, Color::Black);
      }
    }
  }

#if defined(ENABLE_DEBUG)
  dbgUART.println(F("* transitDisplay() - done"));
  dbgUART.println();
#endif
}

int calculateMinutes(const char* expected, unsigned long currentTime) {
  int year, month, day, hour, minute, second;
  sscanf(expected, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);

  struct tm timeinfo = { 0 };
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

// Function to update transit data periodically
void updateTransitData() {
  // This function can be called periodically to refresh the data
  // We're already doing this in the transitDisplay function
  timeClient.update();
}

// Helper function to extract time part from ISO timestamp
String extractTimeFromISO(const char* isoTime) {
  // Format: 2025-04-11T09:12:00
  // We want: 09:12
  char timeBuffer[6];
  int hour, minute;
  sscanf(isoTime, "%*d-%*d-%*dT%d:%d", &hour, &minute);
  sprintf(timeBuffer, "%02d:%02d", hour, minute);
  return String(timeBuffer);
}

// Get color for different transport types
Color getTransportColor(const char* transportMode) {
  if (strcmp(transportMode, "METRO") == 0) {
    return Color::Green;
  } else if (strcmp(transportMode, "BUS") == 0) {
    if (strstr(transportMode, "Blåbuss") != NULL) {
      return Color::Cyan;
    }
    return Color::Yellow;
  } else if (strcmp(transportMode, "TRAIN") == 0) {
    return Color::Orange;
  } else if (strcmp(transportMode, "TRAM") == 0) {
    return Color::Pink;
  }
  return Color::White;
}

void
loop()
{
  tft.clearScreen(Color::Black);
  tft.clearScreen(Color::White);
  tft.clearScreen(Color::Black);
  transitDisplay();
}
