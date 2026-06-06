#include <presentation/menu_presenter.h>

#include <cstdio>
#include <ui/menu.h>

namespace {
MenuWifiIconState menuWifiIconState(MenuPresenterWifiState state) {
  switch (state) {
  case MenuPresenterWifiState::Connected:
    return MenuWifiIconState::Connected;
  case MenuPresenterWifiState::Connecting:
    return MenuWifiIconState::Connecting;
  case MenuPresenterWifiState::Disconnected:
  default:
    return MenuWifiIconState::Disconnected;
  }
}

MenuView buildMenuView(const MenuPresenterState &state) {
  static char time[6];

  MenuView view;
  view.title = "MENU";
  view.items = state.items;
  view.itemCount = state.itemCount;
  view.selectedIndex = state.selectedIndex;
  view.showWifiIcon = true;
  view.wifiIconState = menuWifiIconState(state.wifiState);
  view.wifiIconPhase = state.wifiPhase;

  if (state.hasTime) {
    if (!state.timeValid) {
      view.rightText = "--:--";
    } else {
      std::snprintf(time, sizeof(time), "%02d:%02d", state.hour, state.minute);
      view.rightText = time;
    }
  }

  return view;
}

MenuView buildSettingsMenuView(const SettingsMenuPresenterState &state) {
  MenuView view;
  view.title = "SETTINGS";
  view.items = state.items;
  view.itemCount = state.itemCount;
  view.selectedIndex = state.selectedIndex;
  return view;
}
} // namespace

void menuPresent(const MenuPresenterState &state) {
  drawMenu(buildMenuView(state));
}

void settingsMenuPresent(const SettingsMenuPresenterState &state) {
  drawSettingsMenu(buildSettingsMenuView(state));
}
