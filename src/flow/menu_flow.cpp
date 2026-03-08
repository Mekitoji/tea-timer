#include "flow/menu_flow.h"

#include <Arduino.h>
#include <app/app_state.h>
#include <flow/session_flow.h>
#include <flow/timer_flow.h>
#include <ui.h>

// ----------- menu select handler ----------
void handleMenuSelect() {
  switch (selected) {
  case MENU_TIMER:
    currentScreen = SCREEN_TIMER;
    applyTimerPresetSec(timerDuration);
    resetSingleTimerRuntimeState();
    timerIgnoreReleaseAfterEnter = true;
    drawTimerScreen("Timer", editTimeValue, timerTotalSec);
    break;

  case MENU_SESSION:
    currentScreen = SCREEN_SESSION_PRESET;
    drawSessionPresetMenu();
    break;

  case MENU_SETTINGS:
    currentScreen = SCREEN_SETTINGS;
    settingsSelected = 0;
    drawSettingsMenu();
    break;

  default:
    break;
  }
}

// ----------- settings select handler ----------
void handleSettingsSelect() {
  if (settingsSelected == SETTINGS_WIFI) {
    currentScreen = SCREEN_WIFI;
    drawWiFi();
  } else if (settingsSelected == SETTINGS_ABOUT) {
    currentScreen = SCREEN_ABOUT;
    drawAbout();
  } else if (settingsSelected == SETTINGS_POWER_SAVE) {
    currentScreen = SCREEN_POWER_SAVE;
    powerSaveEditEnabled = powerSaveEnabled;
    drawPowerSave(powerSaveEditEnabled);
    return;
  }
}

// ----------- go back to menu ----------
void goToMenu() {
  currentScreen = SCREEN_MENU;
  drawMenu();
}
