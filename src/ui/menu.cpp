#include <ui/menu.h>

#include <Arduino.h>
#include <hw/display.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
constexpr int MENU_WIFI_ICON_X = 86;
constexpr int MENU_WIFI_ICON_Y = 0;

const uint8_t WIFI_ICON_CONNECTED[] PROGMEM = {
    0b00000000, 0b01111110, 0b10000001, 0b00111100,
    0b01000010, 0b00011000, 0b00011000, 0b00000000,
};

const uint8_t WIFI_ICON_CONNECTING_1[] PROGMEM = {
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00011000, 0b00011000, 0b00000000,
};

const uint8_t WIFI_ICON_CONNECTING_2[] PROGMEM = {
    0b00000000, 0b00000000, 0b00000000, 0b00111100,
    0b01000010, 0b00011000, 0b00011000, 0b00000000,
};

const uint8_t WIFI_ICON_DISCONNECTED[] PROGMEM = {
    0b10000000, 0b01111110, 0b10100001, 0b00111100,
    0b01010010, 0b00011010, 0b00011100, 0b00000010,
};

void drawMenuWifiIcon(MenuWifiIconState state, int phase) {
  const uint8_t *icon = WIFI_ICON_DISCONNECTED;
  if (state == MenuWifiIconState::Connected) {
    icon = WIFI_ICON_CONNECTED;
  } else if (state == MenuWifiIconState::Connecting) {
    if (phase == 0)
      icon = WIFI_ICON_CONNECTING_1;
    else if (phase == 1)
      icon = WIFI_ICON_CONNECTING_2;
    else
      icon = WIFI_ICON_CONNECTED;
  }

  display.drawBitmap(MENU_WIFI_ICON_X, MENU_WIFI_ICON_Y, icon, 8, 8,
                     SSD1306_WHITE);
}

void drawMenuItems(const MenuView &view) {
  display.setTextSize(1);
  for (int i = 0; i < view.itemCount; i++) {
    int y = ui::layout::MENU_LIST_START_Y + i * ui::layout::MENU_LIST_STEP_Y;

    if (i == view.selectedIndex) {
      display.fillRect(ui::layout::MENU_ITEM_BG_X, y - 1,
                       ui::layout::MENU_ITEM_BG_W, ui::layout::MENU_ITEM_H,
                       SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(ui::layout::MENU_ITEM_X, y);
    display.print(view.items[i]);
  }
}
} // namespace

void drawMenu(const MenuView &view) {
  display.clearDisplay();

  drawHeader(view.title, view.rightText);
  if (view.showWifiIcon)
    drawMenuWifiIcon(view.wifiIconState, view.wifiIconPhase);
  drawMenuItems(view);

  display.display();
}

void drawSettingsMenu(const MenuView &view) {
  display.clearDisplay();

  drawHeader(view.title, view.rightText);
  drawMenuItems(view);

  display.display();
}
