#include <controllers/session_controller.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <flow/session_flow.h>
#include <hw/pins.h>
#include <ui.h>

bool handleSessionEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen == SCREEN_SESSION_PRESET) {
    if (SESSION_PRESET_COUNT > 0) {
      app.session.presetIndex += stepPlus ? 1 : -1;
      if (app.session.presetIndex < 0)
        app.session.presetIndex = SESSION_PRESET_COUNT - 1;
      if (app.session.presetIndex >= SESSION_PRESET_COUNT)
        app.session.presetIndex = 0;
      drawSessionPresetMenu();
    }

    return true;
  }

  if (currentScreen == SCREEN_SESSION_RUN) {
    if (app.session.endConfirm.active) {
      if (stepPlus)
        setConfirmChoice(app.session.endConfirm, true);
      if (stepMinus)
        setConfirmChoice(app.session.endConfirm, false);
      drawSessionRun(app.session.stepDurationSec);

      return true;
    }

    if (!isSessionRunning() &&
        (app.session.rinseActive || app.session.stepIndex < app.session.stepCount)) {
      app.session.stepDurationSec += stepPlus ? 1 : -1;
      if (app.session.stepDurationSec < MIN_TIME)
        app.session.stepDurationSec = MIN_TIME;
      if (app.session.stepDurationSec > MAX_TIME)
        app.session.stepDurationSec = MAX_TIME;
      if (app.session.rinseActive) {
        app.session.rinseSec = app.session.stepDurationSec;
      } else if (app.session.stepIndex >= 0 && app.session.stepIndex < app.session.stepCount) {
        app.session.steps[app.session.stepIndex] = app.session.stepDurationSec;
      }
      app.session.stepTotalSec = app.session.stepDurationSec;
      drawSessionRun(app.session.stepTotalSec);
    }

    return true;
  }

  return false;
}

bool handleSessionBackInput() {
  if (currentScreen == SCREEN_SESSION_PRESET) {
    closeConfirm(app.session.endConfirm);
    navigateTo(SCREEN_MENU);
    drawMenu();
    return true;
  }

  if (currentScreen == SCREEN_SESSION_RUN) {
    if (app.session.endConfirm.active) {
      closeConfirm(app.session.endConfirm);
      drawSessionRun(app.session.stepDurationSec);
      return true;
    }

    const bool hasActiveStep =
        app.session.rinseActive || app.session.stepIndex < app.session.stepCount;
    if (!isSessionCompleted() && hasActiveStep) {
      if (isSessionRunning()) {
        sessionToggleRunPauseAt(millis());
      }
      openConfirm(app.session.endConfirm);
      drawSessionRun(app.session.stepDurationSec);
      return true;
    }

    closeConfirm(app.session.endConfirm);
    setSessionStateStopped();
    app.session.rinseActive = (app.session.rinseSec > 0);
    app.session.stepIndex = 0;
    if (app.session.rinseActive) {
      app.session.stepDurationSec = app.session.rinseSec;
      if (app.session.stepDurationSec < MIN_TIME)
        app.session.stepDurationSec = MIN_TIME;
      if (app.session.stepDurationSec > MAX_TIME)
        app.session.stepDurationSec = MAX_TIME;
    } else if (app.session.stepCount > 0) {
      app.session.stepDurationSec = app.session.steps[0];
      if (app.session.stepDurationSec < MIN_TIME)
        app.session.stepDurationSec = MIN_TIME;
      if (app.session.stepDurationSec > MAX_TIME)
        app.session.stepDurationSec = MAX_TIME;
    } else {
      app.session.stepDurationSec = MIN_TIME;
    }
    app.session.stepTotalSec = app.session.stepDurationSec;
    navigateTo(SCREEN_SESSION_PRESET);
    drawSessionPresetMenu();
    return true;
  }

  return false;
}

bool handleSessionSelectInput() {
  if (currentScreen == SCREEN_SESSION_PRESET) {
    loadSessionPresetByIndex(app.session.presetIndex);
    enterSessionRunFromCurrentPreset();

    return true;
  }

  if (currentScreen == SCREEN_SESSION_RUN) {
    if (app.session.endConfirm.active) {
      if (app.session.endConfirm.yesSelected) {
        closeConfirm(app.session.endConfirm);
        app.session.rinseActive = false;
        setSessionStateCompleted();
        app.session.stepIndex = app.session.stepCount;
        drawSessionComplete();
      } else {
        closeConfirm(app.session.endConfirm);
        drawSessionRun(app.session.stepDurationSec);
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
  if (app.session.endConfirm.active) {
    resetSessionLongPressFlowState();
    return;
  }

  const bool down = (digitalRead(ENC_SW) == LOW);
  processSessionLongPressInput(down, millis());
}
