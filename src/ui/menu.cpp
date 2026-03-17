#include <ui/menu.h>

#include <app/app_state.h>
#include <ctime>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
bool readCurrentMenuTime(int &hour, int &minute) {
  time_t now = std::time(nullptr);
  if (now <= 0)
    return false;

  std::tm tmValue = {};
  localtime_r(&now, &tmValue);
  hour = tmValue.tm_hour;
  minute = tmValue.tm_min;
  return true;
}

void drawMenuHeader() {
  char timeBuf[6] = {};
  int hour = 0;
  int minute = 0;
  if (readCurrentMenuTime(hour, minute)) {
    std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    drawHeader("MENU", timeBuf);
    return;
  }

  drawHeader("MENU");
}
} // namespace

void drawMenu() {
  display.clearDisplay();

  drawMenuHeader();

  // items in blue zone
  display.setTextSize(1);
  for (int i = 0; i < menuCount; i++) {
    int y = ui::layout::MENU_LIST_START_Y + i * ui::layout::MENU_LIST_STEP_Y;

    if (i == app.ui.menuSelected) {
      display.fillRect(ui::layout::MENU_ITEM_BG_X, y - 1,
                       ui::layout::MENU_ITEM_BG_W, ui::layout::MENU_ITEM_H,
                       SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(ui::layout::MENU_ITEM_X, y);
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
    int y = ui::layout::MENU_LIST_START_Y + i * ui::layout::MENU_LIST_STEP_Y;

    if (i == app.ui.settingsSelected) {

      display.fillRect(ui::layout::MENU_ITEM_BG_X, y - 1,
                       ui::layout::MENU_ITEM_BG_W, ui::layout::MENU_ITEM_H,
                       SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(ui::layout::MENU_ITEM_X, y);
    display.print(settingsItems[i]);
  }

  display.display();
}

void updateMenuClock() {
  if (currentScreen != SCREEN_MENU)
    return;

  static int lastMinute = -1;
  static int lastHour = -1;

  int hour = 0;
  int minute = 0;
  if (!readCurrentMenuTime(hour, minute))
    return;

  if (hour == lastHour && minute == lastMinute)
    return;

  lastHour = hour;
  lastMinute = minute;
  drawMenu();
}
