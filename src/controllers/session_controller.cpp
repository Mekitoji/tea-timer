#include <controllers/session_controller.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/menu_flow.h>
#include <flow/power_flow.h>
#include <flow/session_flow.h>
#include <hw/pins.h>
#include <ui.h>

bool handleSessionEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen == SCREEN_SESSION_PRESET) {
    if (SESSION_PRESET_COUNT > 0) {
      sessionPresetIndex += stepPlus ? 1 : -1;
      if (sessionPresetIndex < 0)
        sessionPresetIndex = SESSION_PRESET_COUNT - 1;
      if (sessionPresetIndex >= SESSION_PRESET_COUNT)
        sessionPresetIndex = 0;
      drawSessionPresetMenu();
    }

    return true;
  }

  if (currentScreen == SCREEN_SESSION_RUN) {
    if (sessionEndConfirmActive) {
      if (stepPlus)
        sessionEndConfirmYes = true;
      if (stepMinus)
        sessionEndConfirmYes = false;
      drawSessionRun(sessionStepDurationSec);

      return true;
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

    return true;
  }

  return false;
}

bool handleSessionBackInput() {
  if (currentScreen == SCREEN_SESSION_PRESET) {
    sessionEndConfirmActive = false;
    sessionEndConfirmYes = false;
    goToMenu();
    return true;
  }

  if (currentScreen == SCREEN_SESSION_RUN) {
    if (sessionEndConfirmActive) {
      sessionEndConfirmActive = false;
      sessionEndConfirmYes = false;
      drawSessionRun(sessionStepDurationSec);
      return true;
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
      return true;
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
    return true;
  }

  return false;
}

bool handleSessionSelectInput() {
  if (currentScreen == SCREEN_SESSION_PRESET) {
    loadSessionPresetByIndex(sessionPresetIndex);
    enterSessionRunFromCurrentPreset();

    return true;
  }

  if (currentScreen == SCREEN_SESSION_RUN) {
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

    return true;
  }

  return false;
}

void handleSessionLongPressInput() {
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
