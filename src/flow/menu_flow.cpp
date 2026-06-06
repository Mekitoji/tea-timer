#include <flow/menu_flow.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <ctime>
#include <flow/wifi_flow.h>
#include <presentation/menu_presenter.h>

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

MenuPresenterWifiState readMenuWifiState() {
  WifiFlowSnapshot wifi = wifiFlowSnapshot();
  if (wifi.connected)
    return MenuPresenterWifiState::Connected;
  if (wifi.staState == WifiStaUiState::Connecting ||
      wifi.provisionState == WifiProvisionUiState::Connecting)
    return MenuPresenterWifiState::Connecting;
  return MenuPresenterWifiState::Disconnected;
}

int menuWifiIconPhase(MenuPresenterWifiState state) {
  if (state != MenuPresenterWifiState::Connecting)
    return 0;
  return (millis() / MENU_WIFI_CONNECTING_PHASE_MS) % 3;
}

MenuPresenterState menuPresenterState() {
  MenuPresenterState state;
  state.items = menuItems;
  state.itemCount = menuCount;
  state.selectedIndex = app.ui.menuSelected;
  state.timeValid = app.clock.timeValid;
  state.hasTime = readCurrentMenuTime(state.hour, state.minute);
  state.wifiState = readMenuWifiState();
  state.wifiPhase = menuWifiIconPhase(state.wifiState);
  return state;
}

SettingsMenuPresenterState settingsMenuPresenterState() {
  SettingsMenuPresenterState state;
  state.items = settingsItems;
  state.itemCount = settingsMenuCount;
  state.selectedIndex = app.ui.settingsSelected;
  return state;
}
} // namespace

void menuRender() { menuPresent(menuPresenterState()); }

void settingsMenuRender() { settingsMenuPresent(settingsMenuPresenterState()); }

void updateMenuClock() {
  if (currentScreen != SCREEN_MENU)
    return;

  static int lastMinute = -1;
  static int lastHour = -1;
  static bool lastFreshThisBoot = false;
  static bool hasLastWifiIcon = false;
  static MenuPresenterWifiState lastWifiState =
      MenuPresenterWifiState::Disconnected;
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
    MenuPresenterWifiState wifiState = readMenuWifiState();
    int wifiIconPhase = menuWifiIconPhase(wifiState);

    if (!hasLastWifiIcon || wifiState != lastWifiState ||
        wifiIconPhase != lastWifiIconPhase) {
      lastWifiState = wifiState;
      lastWifiIconPhase = wifiIconPhase;
      hasLastWifiIcon = true;
      shouldDraw = true;
    }
  }

  if (shouldDraw)
    menuRender();
}
