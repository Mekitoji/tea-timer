#include <ui/menu.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <ctime>
#include <flow/wifi_flow.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
enum class MenuWifiIconState : uint8_t {
  Disconnected,
  Connecting,
  Connected,
};

constexpr int MENU_WIFI_ICON_X = 86;
constexpr int MENU_WIFI_ICON_Y = 0;
constexpr unsigned long MENU_WIFI_STATUS_POLL_MS = 250;
constexpr unsigned long MENU_WIFI_CONNECTING_PHASE_MS = 350;

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

MenuWifiIconState readMenuWifiIconState() {
  WifiFlowSnapshot wifi = wifiFlowSnapshot();
  if (wifi.connected)
    return MenuWifiIconState::Connected;
  if (wifi.staState == WifiStaUiState::Connecting ||
      wifi.provisionState == WifiProvisionUiState::Connecting)
    return MenuWifiIconState::Connecting;
  return MenuWifiIconState::Disconnected;
}

uint8_t menuWifiIconPhase(MenuWifiIconState state) {
  if (state != MenuWifiIconState::Connecting)
    return 0;
  return (millis() / MENU_WIFI_CONNECTING_PHASE_MS) % 3;
}

void drawMenuWifiIcon(MenuWifiIconState state) {
  const uint8_t *icon = WIFI_ICON_DISCONNECTED;
  if (state == MenuWifiIconState::Connected) {
    icon = WIFI_ICON_CONNECTED;
  } else if (state == MenuWifiIconState::Connecting) {
    uint8_t phase = menuWifiIconPhase(state);
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

void drawMenuHeader() {
  char timeBuf[6] = {};
  int hour = 0;
  int minute = 0;
  if (readCurrentMenuTime(hour, minute)) {
    if (!app.clock.timeValid) {
      drawHeader("MENU", "--:--");
      drawMenuWifiIcon(readMenuWifiIconState());
      return;
    }

    std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
    drawHeader("MENU", timeBuf);
    drawMenuWifiIcon(readMenuWifiIconState());
    return;
  }

  drawHeader("MENU");
  drawMenuWifiIcon(readMenuWifiIconState());
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
  static bool lastFreshThisBoot = false;
  static bool hasLastWifiIcon = false;
  static MenuWifiIconState lastWifiIconState = MenuWifiIconState::Disconnected;
  static uint8_t lastWifiIconPhase = 0;
  static unsigned long lastWifiPollMs = 0;

  int hour = 0;
  int minute = 0;
  bool shouldDraw = false;

  if (readCurrentMenuTime(hour, minute) &&
      (hour != lastHour || minute != lastMinute ||
       app.clock.timeFreshThisBoot != lastFreshThisBoot)) {
    lastHour = hour;
    lastMinute = minute;
    lastFreshThisBoot = app.clock.timeFreshThisBoot;
    shouldDraw = true;
  }

  unsigned long now = millis();
  if (now - lastWifiPollMs >= MENU_WIFI_STATUS_POLL_MS) {
    lastWifiPollMs = now;
    MenuWifiIconState wifiIconState = readMenuWifiIconState();
    uint8_t wifiIconPhase = menuWifiIconPhase(wifiIconState);

    if (!hasLastWifiIcon || wifiIconState != lastWifiIconState ||
        wifiIconPhase != lastWifiIconPhase) {
      lastWifiIconState = wifiIconState;
      lastWifiIconPhase = wifiIconPhase;
      hasLastWifiIcon = true;
      shouldDraw = true;
    }
  }

  if (shouldDraw)
    drawMenu();
}
