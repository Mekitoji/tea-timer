#include <ui/menu.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <ui/header.h>
#include <ui/layout.h>

void drawMenu() {
  display.clearDisplay();

  drawHeader("MENU");

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
