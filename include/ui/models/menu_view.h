#pragma once

enum class MenuWifiIconState {
  Disconnected,
  Connecting,
  Connected,
};

struct MenuView {
  const char *title = "";
  const char *rightText = "";
  const char *const *items = nullptr;
  int itemCount = 0;
  int selectedIndex = 0;
  bool showWifiIcon = false;
  MenuWifiIconState wifiIconState = MenuWifiIconState::Disconnected;
  int wifiIconPhase = 0;
};
