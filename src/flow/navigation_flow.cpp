#include <flow/navigation_flow.h>

#include <app/app_state.h>
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

void showWiFiScreen() {
  navigateTo(SCREEN_WIFI);
  drawWiFi();
}

void showPowerSaveScreen() {
  navigateTo(SCREEN_POWER_SAVE);
  app.power.editEnabled = app.power.enabled;
  drawPowerSave(app.power.editEnabled);
}

bool goBack() {
  switch (currentScreen) {
  case SCREEN_ABOUT:
  case SCREEN_POWER_SAVE:
  case SCREEN_WIFI:
    navigateTo(SCREEN_SETTINGS);
    return true;
  case SCREEN_SETTINGS:
  case SCREEN_TIMER:
  case SCREEN_SESSION_PRESET:
    navigateTo(SCREEN_MENU);
    return true;
  case SCREEN_SESSION_RUN:
    navigateTo(SCREEN_SESSION_PRESET);
    return true;
  default:
    return false;
  }
}

bool goBackAndRender() {
  switch (currentScreen) {
  case SCREEN_ABOUT:
  case SCREEN_POWER_SAVE:
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
