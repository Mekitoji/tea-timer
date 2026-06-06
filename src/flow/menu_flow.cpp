#include <flow/menu_flow.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <cstdio>
#include <ctime>
#include <flow/wifi_flow.h>
#include <ui/menu.h>

namespace {
constexpr unsigned long MENU_WIFI_STATUS_POLL_MS = 250;
constexpr unsigned long MENU_WIFI_CONNECTING_PHASE_MS = 350;

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

int menuWifiIconPhase(MenuWifiIconState state) {
  if (state != MenuWifiIconState::Connecting)
    return 0;
  return (millis() / MENU_WIFI_CONNECTING_PHASE_MS) % 3;
}

MenuView buildMenuView() {
  static char timeBuf[6];

  MenuView view;
  view.title = "MENU";
  view.items = menuItems;
  view.itemCount = menuCount;
  view.selectedIndex = app.ui.menuSelected;
  view.showWifiIcon = true;
  view.wifiIconState = readMenuWifiIconState();
  view.wifiIconPhase = menuWifiIconPhase(view.wifiIconState);

  int hour = 0;
  int minute = 0;
  if (readCurrentMenuTime(hour, minute)) {
    if (!app.clock.timeValid) {
      view.rightText = "--:--";
    } else {
      std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
      view.rightText = timeBuf;
    }
  }

  return view;
}

MenuView buildSettingsMenuView() {
  MenuView view;
  view.title = "SETTINGS";
  view.items = settingsItems;
  view.itemCount = settingsMenuCount;
  view.selectedIndex = app.ui.settingsSelected;
  return view;
}
} // namespace

void menuRender() { drawMenu(buildMenuView()); }

void settingsMenuRender() { drawSettingsMenu(buildSettingsMenuView()); }

void updateMenuClock() {
  if (currentScreen != SCREEN_MENU)
    return;

  static int lastMinute = -1;
  static int lastHour = -1;
  static bool lastFreshThisBoot = false;
  static bool hasLastWifiIcon = false;
  static MenuWifiIconState lastWifiIconState = MenuWifiIconState::Disconnected;
  static int lastWifiIconPhase = 0;
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
    int wifiIconPhase = menuWifiIconPhase(wifiIconState);

    if (!hasLastWifiIcon || wifiIconState != lastWifiIconState ||
        wifiIconPhase != lastWifiIconPhase) {
      lastWifiIconState = wifiIconState;
      lastWifiIconPhase = wifiIconPhase;
      hasLastWifiIcon = true;
      shouldDraw = true;
    }
  }

  if (shouldDraw)
    menuRender();
}
