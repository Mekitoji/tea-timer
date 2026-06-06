#include <flow/navigation_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <flow/audio_settings_flow.h>
#include <flow/clock_flow.h>
#include <flow/menu_flow.h>
#include <flow/power_settings_flow.h>
#include <flow/session_flow.h>
#include <flow/session_history_flow.h>
#include <flow/wifi_flow.h>
#include <presentation/wifi_presenter.h>
#include <ui/settings/about.h>

void navigateTo(ScreenState screen) {
  if (currentScreen == screen)
    return;

  if (currentScreen == SCREEN_WIFI)
    wifiFlowExitScreen();

  currentScreen = screen;

  if (currentScreen == SCREEN_WIFI)
    wifiFlowEnterScreen();
}

void showMenuScreen() {
  navigateTo(SCREEN_MENU);
  menuRender();
}

void showSettingsScreen() {
  navigateTo(SCREEN_SETTINGS);
  settingsMenuRender();
}

void showClockScreen() {
  navigateTo(SCREEN_CLOCK);
  clockRender();
}

void showAboutScreen() {
  navigateTo(SCREEN_ABOUT);
  AboutView view;
  view.chip = "ESP32-C3";
  view.flashMb = ESP.getFlashChipSize() / 1024 / 1024;
  view.freeHeap = ESP.getFreeHeap();
  view.firmwareVersion = appcfg::FIRMWARE_VERSION;
  drawAbout(view);
}

void showAudioScreen() {
  navigateTo(SCREEN_AUDIO);
  audioSettingsRender();
}

void showWiFiScreen() {
  navigateTo(SCREEN_WIFI);
  wifiRender();
}

void showPowerSaveScreen() {
  navigateTo(SCREEN_POWER_SAVE);
  powerSettingsRender();
}

void showSessionHistoryScreen() {
  navigateTo(SCREEN_SESSION_HISTORY);
  sessionHistoryRender();
}

bool goBackAndRender() {
  switch (currentScreen) {
  case SCREEN_ABOUT:
  case SCREEN_POWER_SAVE:
  case SCREEN_AUDIO:
  case SCREEN_CLOCK:
  case SCREEN_WIFI:
    showSettingsScreen();
    return true;
  case SCREEN_SESSION_HISTORY:
    showMenuScreen();
    return true;
  case SCREEN_SETTINGS:
  case SCREEN_TIMER:
  case SCREEN_SESSION_PRESET:
    showMenuScreen();
    return true;
  case SCREEN_SESSION_RUN:
    navigateTo(SCREEN_SESSION_PRESET);
    sessionPresetRender();
    return true;
  default:
    return false;
  }
}
