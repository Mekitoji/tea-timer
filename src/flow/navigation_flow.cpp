#include <flow/navigation_flow.h>

#include <app/app_state.h>
#include <flow/power_settings_flow.h>
#include <flow/audio_settings_flow.h>
#include <ui.h>

void navigateTo(ScreenState screen) { currentScreen = screen; }

void showMenuScreen() {
  navigateTo(SCREEN_MENU);
  drawMenu();
}

void showSettingsScreen() {
  navigateTo(SCREEN_SETTINGS);
  drawSettingsMenu();
}

void showAboutScreen() {
  navigateTo(SCREEN_ABOUT);
  drawAbout();
}

void showAudioScreen() {
  navigateTo(SCREEN_AUDIO);
  audioSettingsRender();
}

void showWiFiScreen() {
  navigateTo(SCREEN_WIFI);
  drawWiFi();
}

void showPowerSaveScreen() {
  navigateTo(SCREEN_POWER_SAVE);
  powerSettingsRender();
}

bool goBackAndRender() {
  switch (currentScreen) {
  case SCREEN_ABOUT:
  case SCREEN_POWER_SAVE:
  case SCREEN_AUDIO:
  case SCREEN_WIFI:
    showSettingsScreen();
    return true;
  case SCREEN_SETTINGS:
  case SCREEN_TIMER:
  case SCREEN_SESSION_PRESET:
    showMenuScreen();
    return true;
  case SCREEN_SESSION_RUN:
    navigateTo(SCREEN_SESSION_PRESET);
    drawSessionPresetMenu();
    return true;
  default:
    return false;
  }
}
