#include <ui/menu.h>

#include <Arduino.h>
#include <app/app_state.h>

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
