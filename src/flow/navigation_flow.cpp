#include <flow/navigation_flow.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <flow/menu_flow.h>
#include <flow/session_flow.h>
#include <flow/session_history_flow.h>
#include <flow/wifi_flow.h>
#include <presentation/about_presenter.h>
#include <presentation/audio_settings_presenter.h>
#include <presentation/clock_presenter.h>
#include <presentation/power_settings_presenter.h>
#include <presentation/session_presenter.h>
#include <presentation/wifi_presenter.h>

namespace {
constexpr unsigned long ABOUT_POLL_MS = 1000;
unsigned long lastAboutUptimeSeconds = 0;
unsigned long lastAboutPollMs = 0;

void renderAboutScreen() {
  lastAboutUptimeSeconds = millis() / 1000UL;
  aboutRender(ESP.getFreeHeap());
}
} // namespace

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
  lastAboutPollMs = millis();
  renderAboutScreen();
}

void updateAboutScreen() {
  if (currentScreen != SCREEN_ABOUT) {
    lastAboutPollMs = 0;
    return;
  }

  unsigned long now = millis();
  if (now - lastAboutPollMs < ABOUT_POLL_MS)
    return;
  lastAboutPollMs = now;

  unsigned long uptimeSeconds = now / 1000UL;
  if (uptimeSeconds == lastAboutUptimeSeconds)
    return;

  renderAboutScreen();
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
  sessionHistoryEnter();
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
