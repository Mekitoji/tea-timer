#include "app/app_controller.h"

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/menu_flow.h>
#include <flow/session_flow.h>
#include <flow/timer_flow.h>
#include <hw/input.h>
#include <hw/pins.h>
#include <ui.h>

void handleEncoderByScreen(bool stepPlus, bool stepMinus) {
  if (stepPlus || stepMinus) {
    if (currentScreen == SCREEN_MENU) {
      selected += stepPlus ? 1 : -1;
      if (selected < 0)
        selected = menuCount - 1;
      if (selected >= menuCount)
        selected = 0;
      drawMenu();
    } else if (currentScreen == SCREEN_TIMER) {
      int step = encoderAccelStepForTimestamp(millis());
      int delta = stepPlus ? step : -step;
      timerAdjustByEncoderDelta(delta);
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
      if (!isSessionRunning() && sessionStepIndex < SESSION_STEP_COUNT) {
        sessionStepDurationSec += stepPlus ? 1 : -1;
        if (sessionStepDurationSec < 1)
          sessionStepDurationSec = 1;
        if (sessionStepDurationSec > MAX_TIME)
          sessionStepDurationSec = MAX_TIME;
        sessionStepTotalSec = sessionStepDurationSec;
        drawSessionRun(sessionStepTotalSec);
      }
    } else if (currentScreen == SCREEN_SETTINGS) {
      settingsSelected += stepPlus ? 1 : -1;
      if (settingsSelected < 0)
        settingsSelected = settingsMenuCount - 1;
      if (settingsSelected >= settingsMenuCount)
        settingsSelected = 0;
      drawSettingsMenu();
    }
  }
}

void handleBackButton() {
  if (!backButtonPressedEvent())
    return;

  if (currentScreen == SCREEN_WIFI || currentScreen == SCREEN_ABOUT) {
    currentScreen = SCREEN_SETTINGS;
    drawSettingsMenu();
    return;
  }

  if (currentScreen == SCREEN_TIMER) {
    applyTimerPresetSec(timerTotalSec);
    prefs.putInt(appcfg::PREFS_DURATION_KEY, timerDuration);
    resetSingleTimerRuntimeState();
  }

  if (currentScreen == SCREEN_SESSION_RUN) {
    sessionStopAndExitToMenu();
    return;
  }

  if (currentScreen != SCREEN_MENU) {
    goToMenu();
  }
}

void handleSelectButton() {
  if (buttonPressedEvent()) {
    if (currentScreen == SCREEN_MENU) {
      handleMenuSelect();
    } else if (currentScreen == SCREEN_TIMER) {
      // handled by handleTimerButton
    } else if (currentScreen == SCREEN_SESSION_MENU) {
      goToMenu();
    } else if (currentScreen == SCREEN_SESSION_RUN) {
      sessionToggleRunPauseAt(millis());
    } else if (currentScreen == SCREEN_SETTINGS) {
      handleSettingsSelect();
    } else if (currentScreen == SCREEN_WIFI || currentScreen == SCREEN_ABOUT) {
      currentScreen = SCREEN_SETTINGS;
      drawSettingsMenu();
    } else {
      goToMenu();
    }
  }
}

void handleSessionLongPress() {
  if (currentScreen != SCREEN_SESSION_RUN) {
    resetSessionLongPressFlowState();
    return;
  }

  const bool down = (digitalRead(ENC_SW) == LOW);
  processSessionLongPressInput(down, millis());
}

void handleTimerButton() {
  if (currentScreen != SCREEN_TIMER) {
    timerIgnoreReleaseAfterEnter = false;
    resetTimerButtonFlowState();
    return;
  }

  const unsigned long now = millis();
  const bool down = (digitalRead(ENC_SW) == LOW);
  processTimerButtonInput(down, now);
}
