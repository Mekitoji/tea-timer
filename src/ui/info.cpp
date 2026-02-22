#include <ui/info.h>

#include <Arduino.h>
#include <WiFi.h>
#include <app/app_state.h>

namespace {
bool wifiScanPending = false;
bool wifiResultShown = false;
} // namespace

void drawAbout() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("About Device");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.print("Chip: ESP32-C3");

  display.setCursor(0, 26);
  display.print("Flash: ");
  display.print(ESP.getFlashChipSize() / 1024 / 1024);
  display.print("MB");

  display.setCursor(0, 36);
  display.print("Heap: ");
  display.print(ESP.getFreeHeap());

  display.setCursor(0, 46);
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
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("WiFi Found:");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  for (int i = 0; i < wifiCount && i < 4; i++) {
    display.setCursor(0, 14 + i * 10);
    display.print(WiFi.SSID(i));
  }

  if (wifiCount == 0) {
    display.setCursor(0, 20);
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
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Scanning WiFi...");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.scanDelete();
  WiFi.scanNetworks(true); // async
}
