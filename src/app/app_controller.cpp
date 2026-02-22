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
      } else if (!sessionRunning) {
        sessionRunning = true;
        sessionStepStartMs = millis();
      }
    } else {
      goToMenu();
    }
  }
}

void handleSessionLongPress() {
  if (currentScreen == SCREEN_SESSION_RUN) {
    bool down = (digitalRead(ENC_SW) == LOW);

    if (down && !swWasDown) {
      swWasDown = true;
      swHoldStartMs = millis();
    }

    if (!down && swWasDown) {
      swWasDown = false;
    }

    if (down && swWasDown &&
        (millis() - swHoldStartMs >= appcfg::SESSION_HOLD_MS)) {
      while (digitalRead(ENC_SW) == LOW)
        delay(appcfg::SESSION_RELEASE_POLL_MS);
      delay(appcfg::SESSION_RELEASE_SETTLE_MS);

      sessionRunning = false;
      goToMenu();
    }
  }
}
