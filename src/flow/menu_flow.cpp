#include "flow/menu_flow.h"

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/timer_flow.h>
#include <ui.h>

// ----------- menu select handler ----------
void handleMenuSelect() {
  if (selected == MENU_START_SESSION) {
    sessionStepIndex = 0;
    sessionRunning = false;
    sessionCompleteShown = false;
    currentScreen = SCREEN_SESSION_RUN;
    drawSessionRun(SESSION_STEPS[0]);
  } else if (selected == MENU_SESSION) {
    currentScreen = SCREEN_SESSION_MENU;
    drawSessionMenu();
  } else if (selected == MENU_START) {
    resetSingleTimerFlowState();
    currentScreen = SCREEN_TIMER;
    timerStartMillis = millis();
  } else if (selected == MENU_SET_TIME) {
    currentScreen = SCREEN_SET_TIME;
    editTimeValue = timerDuration;
    drawSetTime();
  } else if (selected == MENU_WIFI) {
    currentScreen = SCREEN_WIFI;
    drawWiFi();
  } else if (selected == MENU_ABOUT) {
    currentScreen = SCREEN_ABOUT;
    drawAbout();
  }
}
