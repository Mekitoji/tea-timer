#include <ui/menu.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <ui/header.h>

void drawMenu() {
  display.clearDisplay();

  drawHeader("MENU");

  // items in blue zone
  display.setTextSize(1);
  for (int i = 0; i < menuCount; i++) {
    int y = 17 + i * 8;

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

void drawSettingsMenu() {
  display.clearDisplay();

  drawHeader("SETTINGS");

  // items in blue zone
  display.setTextSize(1);
  for (int i = 0; i < settingsMenuCount; i++) {
    int y = 17 + i * 8;

    if (i == settingsSelected) {
      display.fillRect(0, y - 1, 128, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(4, y);
    display.print(settingsItems[i]);
  }

  display.display();
}
