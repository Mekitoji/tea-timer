#include "flow/menu_flow.h"

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/session_flow.h>
#include <flow/timer_flow.h>
#include <ui.h>

// ----------- menu select handler ----------
void handleMenuSelect() {
  if (selected == MENU_START_SESSION) {
    resetSessionFlowState();
    sessionStepIndex = 0;
    setSessionStateStopped();
    currentScreen = SCREEN_SESSION_RUN;
    sessionStepDurationSec = SESSION_STEPS[0];
    sessionStepTotalSec = sessionStepDurationSec;
    drawSessionRun(sessionStepDurationSec);
  } else if (selected == MENU_SESSION) {
    currentScreen = SCREEN_SESSION_MENU;
    drawSessionMenu();
  } else if (selected == MENU_TIMER) {
    currentScreen = SCREEN_TIMER;
    applyTimerPresetSec(timerDuration);
    resetSingleTimerRuntimeState();
    timerIgnoreReleaseAfterEnter = true;
    drawTimerScreen("Timer", editTimeValue, timerTotalSec);
  } else if (selected == MENU_SETTINGS) {
    currentScreen = SCREEN_SETTINGS;
    settingsSelected = 0;
    drawSettingsMenu();
  }
}

// ----------- settings select handler ----------
void handleSettingsSelect() {
  if (settingsSelected == SETTINGS_WIFI) {
    currentScreen = SCREEN_WIFI;
    wifiCount = 0;
    drawWiFi();
  } else if (settingsSelected == SETTINGS_ABOUT) {
    currentScreen = SCREEN_ABOUT;
    drawAbout();
  }
}

// ----------- go back to menu ----------
void goToMenu() {
  currentScreen = SCREEN_MENU;
  drawMenu();
}
