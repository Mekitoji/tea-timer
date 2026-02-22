#include "ui.h"
#include <app_state.h>

#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <WiFi.h>
#include <tea_config.h>

void drawProgressBar(int remaining, int total) {
  int x = 6, y = 54, w = 116, h = 8;
  display.drawRect(x, y, w, h, SSD1306_WHITE);

  int elapsed = total - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > total)
    elapsed = total;

  int fill = (total == 0) ? 0 : (elapsed * (w - 2)) / total;
  if (fill < 0)
    fill = 0;
  if (fill > w - 2)
    fill = w - 2;

  display.fillRect(x + 1, y + 1, fill, h - 2, SSD1306_WHITE);
}

void drawMenu() {
  display.clearDisplay();

  // header in yellow zone
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 4);
  display.print("MENU");

  display.drawLine(0, 16, 128, 16, SSD1306_WHITE);

  // items in blue zone
  display.setTextSize(1);
  for (int i = 0; i < menuCount; i++) {
    int y = 22 + i * 7;

    if (i == selected) {
      display.fillRect(0, y - 1, 128, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(4, y);
    display.print(menuItems[i]);
  }

  display.display();
}

void drawTimerScreen(const char *title, int secondsLeft, int totalSeconds) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print(title);
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setTextSize(3);
  display.setCursor(40, 22);
  display.print(secondsLeft);

  display.setTextSize(1);
  drawProgressBar(secondsLeft, totalSeconds);

  display.display();
}

void drawSetTime() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Set Time");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  int mm = editTimeValue / 60;
  int ss = editTimeValue % 60;

  display.setTextSize(2);
  display.setCursor(10, 28);
  if (mm < 10)
    display.print('0');
  display.print(mm);
  display.print(':');
  if (ss < 10)
    display.print('0');
  display.print(ss);

  display.setTextSize(1);
  display.setCursor(0, 54);
  display.print("Press to save");

  display.display();
}

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

void drawWiFi() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Scanning WiFi...");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(200);

  wifiCount = WiFi.scanNetworks();

  display.clearDisplay();
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
}

void drawSessionComplete() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Header (yellow)
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.print("SESSION");
  display.drawLine(0, 14, 128, 14, SSD1306_WHITE);

  // Body
  display.setTextSize(2);
  display.setCursor(8, 24);
  display.print("COMPLETE");

  display.setTextSize(1);
  display.setCursor(0, 42);
  display.print("Tea: ");
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, 54);
  display.print("Press to exit");

  display.display();
}

void drawSessionMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Session");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print("Tea:");
  display.setCursor(30, 18);
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, 34);
  display.print("Steps:");
  display.setCursor(45, 34);
  display.print(SESSION_STEP_COUNT);
  display.print(" infusions");

  display.setCursor(0, 54);
  display.print("Press: back");

  display.display();
}

void drawSessionRun(int remaining) {
  if (sessionStepIndex >= SESSION_STEP_COUNT) {
    drawSessionComplete();
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ---- HEADER ----
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.print("SESSION RUN");

  display.drawLine(0, 14, 128, 14, SSD1306_WHITE);

  // ---- INFO BLOCK ----
  display.setTextSize(1);

  display.setCursor(0, 18);
  display.print("Tea:");
  display.setCursor(28, 18);
  display.print(TEAS[sessionTeaIndex]);

  display.setCursor(0, 28);
  display.print("Step ");
  display.print(sessionStepIndex + 1);
  display.print("/");
  display.print(SESSION_STEP_COUNT);

  display.setCursor(0, 38);
  if (sessionStepIndex == 0)
    display.print("Rinse ");
  else
    display.print("Infuse ");

  display.print(SESSION_STEPS[sessionStepIndex]);
  display.print("s");

  // ---- TIMER BIG ----
  display.setTextSize(2);
  display.setCursor(78, 28);
  display.print(remaining);

  // ---- PROGRESS BAR ----
  int x = 6;
  int y = 50;
  int w = 116;
  int h = 8;

  display.drawRect(x, y, w, h, SSD1306_WHITE);

  int total = SESSION_STEPS[sessionStepIndex];
  int elapsed = total - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > total)
    elapsed = total;

  int fill = (total == 0) ? 0 : (elapsed * (w - 2)) / total;
  display.fillRect(x + 1, y + 1, fill, h - 2, SSD1306_WHITE);

  display.display();
}
