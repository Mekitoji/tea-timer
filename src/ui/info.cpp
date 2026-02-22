#include <ui/info.h>

#include <Arduino.h>
#include <WiFi.h>
#include <app/app_state.h>
#include <ui/header.h>

namespace {
bool wifiScanPending = false;
bool wifiResultShown = false;
} // namespace

void drawAbout() {
  display.clearDisplay();
  drawHeader("About Device");

  display.setCursor(0, UI_HEADER_CONTENT_Y);
  display.print("Chip: ESP32-C3");

  display.setCursor(0, UI_HEADER_CONTENT_Y + 10);
  display.print("Flash: ");
  display.print(ESP.getFlashChipSize() / 1024 / 1024);
  display.print("MB");

  display.setCursor(0, UI_HEADER_CONTENT_Y + 20);
  display.print("Heap: ");
  display.print(ESP.getFreeHeap());

  display.setCursor(0, UI_HEADER_CONTENT_Y + 30);
  display.print("FW: v1.0.0");

  display.display();
}

void updateWiFiScreen() {
  if (!wifiScanPending || wifiResultShown)
    return;

  int scanResult = WiFi.scanComplete();

  if (scanResult == -1)
    return; // still scanning
  if (scanResult == -2) {
    WiFi.scanNetworks(true); // restart if needed
    return;
  }

  wifiCount = scanResult;

  display.clearDisplay();
  drawHeader("WiFi Found:");

  for (int i = 0; i < wifiCount && i < 4; i++) {
    display.setCursor(0, UI_HEADER_CONTENT_Y + 1 + i * 10);
    display.print(WiFi.SSID(i));
  }

  if (wifiCount == 0) {
    display.setCursor(0, UI_HEADER_CONTENT_Y + 5);
    display.print("No networks");
  }

  display.display();

  wifiResultShown = true;
  wifiScanPending = false;
  WiFi.scanDelete();
}

void drawWiFi() {
  wifiResultShown = false;
  wifiScanPending = true;

  display.clearDisplay();
  drawHeader("Wi-Fi");
  display.setCursor(0, UI_HEADER_CONTENT_Y + 6);
  display.print("Scanning...");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.scanDelete();
  WiFi.scanNetworks(true); // async
}
