#pragma once

enum class MenuPresenterWifiState {
  Disconnected,
  Connecting,
  Connected,
};

struct MenuPresenterState {
  const char *const *items = nullptr;
  int itemCount = 0;
  int selectedIndex = 0;
  bool hasTime = false;
  bool timeValid = false;
  int hour = 0;
  int minute = 0;
  MenuPresenterWifiState wifiState = MenuPresenterWifiState::Disconnected;
  int wifiPhase = 0;
};

struct SettingsMenuPresenterState {
  const char *const *items = nullptr;
  int itemCount = 0;
  int selectedIndex = 0;
};

void menuPresent(const MenuPresenterState &state);
void settingsMenuPresent(const SettingsMenuPresenterState &state);
