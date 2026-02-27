#include "app/app_controller.h"

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/menu_flow.h>
#include <flow/power_flow.h>
#include <flow/session_flow.h>
#include <flow/timer_flow.h>
#include <hw/input.h>
#include <hw/pins.h>
#include <ui.h>

void handleEncoderByScreen(bool stepPlus, bool stepMinus) {
  if (stepPlus || stepMinus) {
    if (isWakeInputGuardActive())
      return;
    if (markUserActivityAndConsumeIfWoke())
      return;
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
    } else if (currentScreen == SCREEN_SESSION_PRESET) {
      if (SESSION_PRESET_COUNT > 0) {
        sessionPresetIndex += stepPlus ? 1 : -1;
        if (sessionPresetIndex < 0)
          sessionPresetIndex = SESSION_PRESET_COUNT - 1;
        if (sessionPresetIndex >= SESSION_PRESET_COUNT)
          sessionPresetIndex = 0;
        drawSessionPresetMenu();
      }
    } else if (currentScreen == SCREEN_SESSION_RUN) {
      if (sessionEndConfirmActive) {
        if (stepPlus)
          sessionEndConfirmYes = true;
        if (stepMinus)
          sessionEndConfirmYes = false;
        drawSessionRun(sessionStepDurationSec);
        return;
      }

      if (!isSessionRunning() &&
          (sessionRinseActive || sessionStepIndex < sessionStepCount)) {
        sessionStepDurationSec += stepPlus ? 1 : -1;
        if (sessionStepDurationSec < MIN_TIME)
          sessionStepDurationSec = MIN_TIME;
        if (sessionStepDurationSec > MAX_TIME)
          sessionStepDurationSec = MAX_TIME;
        if (sessionRinseActive) {
          sessionRinseSec = sessionStepDurationSec;
        } else if (sessionStepIndex >= 0 && sessionStepIndex < sessionStepCount) {
          sessionSteps[sessionStepIndex] = sessionStepDurationSec;
        }
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
    } else if (currentScreen == SCREEN_POWER_SAVE) {
      powerSaveEditEnabled = !powerSaveEditEnabled;
      drawPowerSave(powerSaveEditEnabled);
    }
  }
}

void handleBackButton() {
  if (!backButtonPressedEvent())
    return;

  if (isWakeInputGuardActive())
    return;
  if (markUserActivityAndConsumeIfWoke())
    return;

  if (currentScreen == SCREEN_WIFI || currentScreen == SCREEN_ABOUT ||
      currentScreen == SCREEN_POWER_SAVE) {
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
    if (sessionEndConfirmActive) {
      sessionEndConfirmActive = false;
      sessionEndConfirmYes = false;
      drawSessionRun(sessionStepDurationSec);
      return;
    }

    const bool hasActiveStep =
        sessionRinseActive || sessionStepIndex < sessionStepCount;
    if (!isSessionCompleted() && hasActiveStep) {
      if (isSessionRunning()) {
        sessionToggleRunPauseAt(millis());
      }
      sessionEndConfirmActive = true;
      sessionEndConfirmYes = false;
      drawSessionRun(sessionStepDurationSec);
      return;
    }

    sessionEndConfirmActive = false;
    sessionEndConfirmYes = false;
    setSessionStateStopped();
    sessionRinseActive = (sessionRinseSec > 0);
    sessionStepIndex = 0;
    if (sessionRinseActive) {
      sessionStepDurationSec = sessionRinseSec;
      if (sessionStepDurationSec < MIN_TIME)
        sessionStepDurationSec = MIN_TIME;
      if (sessionStepDurationSec > MAX_TIME)
        sessionStepDurationSec = MAX_TIME;
    } else if (sessionStepCount > 0) {
      sessionStepDurationSec = sessionSteps[0];
      if (sessionStepDurationSec < MIN_TIME)
        sessionStepDurationSec = MIN_TIME;
      if (sessionStepDurationSec > MAX_TIME)
        sessionStepDurationSec = MAX_TIME;
    } else {
      sessionStepDurationSec = MIN_TIME;
    }
    sessionStepTotalSec = sessionStepDurationSec;
    currentScreen = SCREEN_SESSION_PRESET;
    drawSessionPresetMenu();
    return;
  }

  if (currentScreen == SCREEN_SESSION_PRESET) {
    sessionEndConfirmActive = false;
    sessionEndConfirmYes = false;
    goToMenu();
    return;
  }

  if (currentScreen != SCREEN_MENU) {
    goToMenu();
  }
}

void handleSelectButton() {
  if (buttonPressedEvent()) {
    if (isWakeInputGuardActive())
      return;
    if (markUserActivityAndConsumeIfWoke())
      return;

    if (currentScreen == SCREEN_MENU) {
      handleMenuSelect();
    } else if (currentScreen == SCREEN_TIMER) {
      // handled by handleTimerButton
    } else if (currentScreen == SCREEN_SESSION_PRESET) {
      loadSessionPresetByIndex(sessionPresetIndex);
      enterSessionRunFromCurrentPreset();
    } else if (currentScreen == SCREEN_SESSION_RUN) {
      if (sessionEndConfirmActive) {
        if (sessionEndConfirmYes) {
          sessionEndConfirmActive = false;
          sessionEndConfirmYes = false;
          sessionRinseActive = false;
          setSessionStateCompleted();
          sessionStepIndex = sessionStepCount;
          drawSessionComplete();
        } else {
          sessionEndConfirmActive = false;
          sessionEndConfirmYes = false;
          drawSessionRun(sessionStepDurationSec);
        }
      } else {
        sessionToggleRunPauseAt(millis());
      }
    } else if (currentScreen == SCREEN_SETTINGS) {
      handleSettingsSelect();
    } else if (currentScreen == SCREEN_WIFI || currentScreen == SCREEN_ABOUT) {
      currentScreen = SCREEN_SETTINGS;
      drawSettingsMenu();
    } else if (currentScreen == SCREEN_POWER_SAVE) {
      powerSaveEnabled = powerSaveEditEnabled;
      setPowerSavingEnabled(powerSaveEnabled);
      prefs.putBool(appcfg::PREFS_POWER_SAVE_KEY, powerSaveEnabled);
      currentScreen = SCREEN_SETTINGS;
      drawSettingsMenu();
    } else {
      goToMenu();
    }
  }
}

void handleSessionLongPress() {
  if (isWakeInputGuardActive())
    return;
  if (currentScreen != SCREEN_SESSION_RUN) {
    resetSessionLongPressFlowState();
    return;
  }
  if (sessionEndConfirmActive) {
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

  if (isWakeInputGuardActive())
    return;

  const unsigned long now = millis();
  const bool down = (digitalRead(ENC_SW) == LOW);
  processTimerButtonInput(down, now);
}
