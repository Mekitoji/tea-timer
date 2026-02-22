#include "app/app_controller.h"

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/menu_flow.h>
#include <hw/input.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
unsigned long swHoldStartMs = 0;
bool swWasDown = false;
unsigned long lastStepMs = 0;
bool sessionLongPressFired = false;
} // namespace

void handleEncoderByScreen(bool stepPlus, bool stepMinus) {
  if (stepPlus || stepMinus) {
    if (currentScreen == SCREEN_MENU) {
      selected += stepPlus ? 1 : -1;
      if (selected < 0)
        selected = menuCount - 1;
      if (selected >= menuCount)
        selected = 0;
      drawMenu();
    } else if (currentScreen == SCREEN_SET_TIME) {
      unsigned long now = millis();
      unsigned long dt = now - lastStepMs;
      lastStepMs = now;

      int step = appcfg::ENC_STEP_NORMAL;
      if (dt < appcfg::ENC_ACCEL_FAST_MS)
        step = appcfg::ENC_STEP_FAST;
      else if (dt < appcfg::ENC_ACCEL_MEDIUM_MS)
        step = appcfg::ENC_STEP_MEDIUM;

      editTimeValue += (stepPlus ? step : -step);
      if (editTimeValue < MIN_TIME)
        editTimeValue = MIN_TIME;
      if (editTimeValue > MAX_TIME)
        editTimeValue = MAX_TIME;

      drawSetTime();
    } else if (currentScreen == SCREEN_SESSION_MENU) {
      if (TEA_COUNT > 1) {
        sessionTeaIndex += stepPlus ? 1 : -1;
        if (sessionTeaIndex < 0)
          sessionTeaIndex = TEA_COUNT - 1;
        if (sessionTeaIndex >= TEA_COUNT)
          sessionTeaIndex = 0;
        drawSessionMenu();
      }
    } else if (currentScreen == SCREEN_SESSION_RUN) {
      if (!sessionRunning && sessionStepIndex < SESSION_STEP_COUNT) {
        sessionStepDurationSec += stepPlus ? 1 : -1;
        if (sessionStepDurationSec < 1)
          sessionStepDurationSec = 1;
        if (sessionStepDurationSec > MAX_TIME)
          sessionStepDurationSec = MAX_TIME;
        sessionStepTotalSec = sessionStepDurationSec;
        drawSessionRun(sessionStepTotalSec);
      }
    }
  }
}

void handleBackButton() {
  if (backButtonPressedEvent()) {
    if (currentScreen != SCREEN_MENU) {
      if (currentScreen == SCREEN_SESSION_RUN)
        sessionRunning = false;
      goToMenu();
    }
  }
}

void handleSelectButton() {
  if (buttonPressedEvent()) {
    if (currentScreen == SCREEN_MENU) {
      handleMenuSelect();
    } else if (currentScreen == SCREEN_SET_TIME) {
      timerDuration = editTimeValue;
      prefs.putInt(appcfg::PREFS_DURATION_KEY, timerDuration);
      goToMenu();
    } else if (currentScreen == SCREEN_SESSION_MENU) {
      goToMenu();
    } else if (currentScreen == SCREEN_SESSION_RUN) {

      if (sessionStepIndex >= SESSION_STEP_COUNT) {
        goToMenu();
      } else if (sessionRunning) {
        int stepSec = sessionStepDurationSec > 0
                          ? sessionStepDurationSec
                          : SESSION_STEPS[sessionStepIndex];
        unsigned long elapsed = (millis() - sessionStepStartMs) / 1000;
        int remaining = stepSec - (int)elapsed;
        if (remaining < 0)
          remaining = 0;

        sessionStepDurationSec = remaining;
        sessionRunning = false;
        drawSessionRun(sessionStepDurationSec);
      } else { // resume or start step
        if (sessionStepDurationSec <= 0)
          sessionStepDurationSec = SESSION_STEPS[sessionStepIndex];
        if (sessionStepTotalSec <= 0)
          sessionStepTotalSec = sessionStepDurationSec;

        sessionRunning = true;
        sessionStepStartMs = millis();
        drawSessionRun(sessionStepDurationSec);
      }
    } else {
      goToMenu();
    }
  }
}

void handleSessionLongPress() {
  if (currentScreen != SCREEN_SESSION_RUN) {
    swWasDown = false;
    sessionLongPressFired = false;
    return;
  }

  bool down = (digitalRead(ENC_SW) == LOW);

  if (down && !swWasDown) {
    swWasDown = true;
    swHoldStartMs = millis();
    sessionLongPressFired = false;
  }

  if (!down && swWasDown) {
    swWasDown = false;
    sessionLongPressFired = false;
    return;
  }

  if (!down || !swWasDown || sessionLongPressFired)
    return;

  if (millis() - swHoldStartMs < appcfg::SESSION_HOLD_MS)
    return;

  sessionLongPressFired = true;

  sessionStepIndex++;
  sessionCompleteShown = false;

  if (sessionStepIndex >= SESSION_STEP_COUNT) {
    sessionRunning = false;
    drawSessionComplete();
    sessionCompleteShown = true;
    return;
  }

  sessionStepDurationSec = SESSION_STEPS[sessionStepIndex];
  sessionStepTotalSec = sessionStepDurationSec;
  sessionRunning = false;
  drawSessionRun(sessionStepDurationSec);
}
