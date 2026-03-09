#include <flow/session_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/session_presets.h>
#include <app/tea_config.h>
#include <flow/audio_profile_flow.h>
#include <flow/navigation_flow.h>
#include <hw/feedback.h>
#include <ui.h>

namespace {
int lastRemaining = -1;

int clampRinseSec(int sec) {
  if (sec <= 0)
    return 0;
  if (sec > MAX_TIME)
    return MAX_TIME;
  return sec;
}

int clampInfusionSec(int sec) {
  if (sec < MIN_TIME)
    return MIN_TIME;
  if (sec > MAX_TIME)
    return MAX_TIME;
  return sec;
}

int stepSecAt(int index) {
  if (index < 0 || index >= app.session.stepCount)
    return MIN_TIME;
  return clampInfusionSec(app.session.steps[index]);
}

bool hasCurrentSessionStep() {
  if (app.session.rinseActive && app.session.rinseSec > 0)
    return true;
  return app.session.stepIndex >= 0 &&
         app.session.stepIndex < app.session.stepCount;
}

void applyCurrentStepFromModel() {
  if (app.session.rinseActive) {
    int rinse = app.session.rinseSec;
    if (rinse < MIN_TIME)
      rinse = MIN_TIME;
    if (rinse > MAX_TIME)
      rinse = MAX_TIME;
    app.session.rinseSec = rinse;
    app.session.stepDurationSec = rinse;
    app.session.stepTotalSec = rinse;
    return;
  }

  app.session.stepDurationSec = stepSecAt(app.session.stepIndex);
  app.session.stepTotalSec = app.session.stepDurationSec;
}

bool advanceToNextSessionStep() {
  if (app.session.rinseActive) {
    app.session.rinseActive = false;
    app.session.stepIndex = 0;
    if (app.session.stepCount <= 0)
      return false;
    applyCurrentStepFromModel();
    return true;
  }

  app.session.stepIndex++;
  if (app.session.stepIndex >= app.session.stepCount)
    return false;

  applyCurrentStepFromModel();
  return true;
}

unsigned long sessionHoldStartMs = 0;
bool sessionWasDown = false;
bool sessionLongPressFired = false;
} // namespace

void resetSessionFlowState() { lastRemaining = -1; }

void resetSessionLongPressFlowState() {
  sessionWasDown = false;
  sessionLongPressFired = false;
}

void processSessionLongPressInput(bool down, unsigned long nowMs) {
  if (down && !sessionWasDown) {
    sessionWasDown = true;
    sessionHoldStartMs = nowMs;
    sessionLongPressFired = false;
  }

  if (!down && sessionWasDown) {
    sessionWasDown = false;
    sessionLongPressFired = false;
    return;
  }

  if (!down || !sessionWasDown || sessionLongPressFired)
    return;

  if (nowMs - sessionHoldStartMs < appcfg::SESSION_HOLD_MS)
    return;

  sessionLongPressFired = true;

  if (!advanceToNextSessionStep()) {
    setSessionStateCompleted();
    drawSessionComplete();
    return;
  }

  setSessionStatePaused();
  drawSessionRun(app.session.stepDurationSec);
}

void loadSessionPresetByIndex(int presetIndex) {
  app.session.endConfirm.active = false;
  app.session.endConfirm.yesSelected = false;

  if (SESSION_PRESET_COUNT <= 0) {
    app.session.presetIndex = 0;
    app.session.stepCount = 0;
    app.session.rinseSec = 0;
    app.session.rinseActive = false;
    setSessionStateStopped();
    resetSessionFlowState();
    return;
  }

  if (presetIndex < 0)
    presetIndex = SESSION_PRESET_COUNT - 1;
  if (presetIndex >= SESSION_PRESET_COUNT)
    presetIndex = 0;

  app.session.presetIndex = presetIndex;

  const SessionPreset &preset = SESSION_PRESETS[app.session.presetIndex];
  app.session.rinseSec = clampRinseSec(preset.rinseSec);
  app.session.rinseActive = (app.session.rinseSec > 0);

  int count = preset.stepCount;
  if (count < 0)
    count = 0;
  if (count > SESSION_MAX_STEPS)
    count = SESSION_MAX_STEPS;

  app.session.stepCount = count;

  for (int i = 0; i < app.session.stepCount; i++) {
    app.session.steps[i] = clampInfusionSec(preset.stepsSec[i]);
  }
  for (int i = app.session.stepCount; i < SESSION_MAX_STEPS; i++) {
    app.session.steps[i] = 0;
  }

  app.session.stepIndex = 0;
  if (app.session.rinseActive) {
    app.session.stepDurationSec = app.session.rinseSec;
  } else if (app.session.stepCount > 0) {
    app.session.stepDurationSec = stepSecAt(0);
  } else {
    app.session.stepDurationSec = MIN_TIME;
  }
  app.session.stepTotalSec = app.session.stepDurationSec;

  setSessionStateStopped();
  resetSessionFlowState();
}

void enterSessionRunFromCurrentPreset() {
  app.session.endConfirm.active = false;
  app.session.endConfirm.yesSelected = false;

  if (app.session.stepCount <= 0) {
    loadSessionPresetByIndex(app.session.presetIndex);
  }

  if (app.session.stepCount <= 0 && app.session.rinseSec <= 0)
    return;

  app.session.stepIndex = 0;
  app.session.rinseActive = (app.session.rinseSec > 0);
  applyCurrentStepFromModel();

  setSessionStatePaused();
  navigateTo(SCREEN_SESSION_RUN);
  drawSessionRun(app.session.stepDurationSec);
}

void updateSessionRun() {
  if (currentScreen == SCREEN_SESSION_RUN) {
    if (!hasCurrentSessionStep()) {
      if (!isSessionCompleted()) {
        setSessionStateCompleted();
        drawSessionComplete();
      }
      return;
    }

    int stepSec = app.session.stepDurationSec;
    if (stepSec <= 0) {
      stepSec = app.session.rinseActive ? app.session.rinseSec
                                        : stepSecAt(app.session.stepIndex);
    }
    if (stepSec < MIN_TIME)
      stepSec = MIN_TIME;
    if (stepSec > MAX_TIME)
      stepSec = MAX_TIME;
    int remaining = stepSec;

    if (isSessionRunning()) {
      unsigned long elapsed = (millis() - app.session.stepStartMs) / 1000;
      remaining = stepSec - (int)elapsed;
      if (remaining < 0)
        remaining = 0;
    }

    if (remaining != lastRemaining) {
      drawSessionRun(remaining);

      if (isSessionRunning() && remaining <= 3 && remaining > 0) {
        pulseLedAndSound(audioProfileCountdownFreq(),
                         audioProfileBeepDurationMs(),
                         app.audio.soundEnabled);
      }
    }

    if (isSessionRunning() && remaining == 0) {
      for (int i = 0; i < 2; i++) {
        pulseLedAndSound(audioProfileSessionStepDoneFreq(),
                         audioProfileBeepDurationMs(),
                         app.audio.soundEnabled);
        delay(120);
      }

      setSessionStatePaused();
      if (!advanceToNextSessionStep()) {
        for (int i = 0; i < 3; i++) {
          pulseLedAndSound(audioProfileSessionDoneFreq(),
                           audioProfileBeepDurationMs(),
                           app.audio.soundEnabled);
          delay(160);
        }

        setSessionStateCompleted();
        app.session.endConfirm.active = false;
        app.session.endConfirm.yesSelected = false;
        drawSessionComplete();
        lastRemaining = -999;
        return;
      }

      drawSessionRun(app.session.stepDurationSec);
      lastRemaining = -1;
      return;
    }

    lastRemaining = remaining;
  }
}

void sessionToggleRunPauseAt(unsigned long nowMs) {
  if (!hasCurrentSessionStep()) {
    navigateTo(SCREEN_MENU);
    drawMenu();

    return;
  }

  if (isSessionRunning()) {
    int stepSec = app.session.stepDurationSec;
    if (stepSec <= 0) {
      stepSec = app.session.rinseActive ? app.session.rinseSec
                                        : stepSecAt(app.session.stepIndex);
    }
    if (stepSec < MIN_TIME)
      stepSec = MIN_TIME;
    if (stepSec > MAX_TIME)
      stepSec = MAX_TIME;

    unsigned long elapsed = (nowMs - app.session.stepStartMs) / 1000;
    int remaining = stepSec - (int)elapsed;
    if (remaining < 0)
      remaining = 0;

    app.session.stepDurationSec = remaining;
    setSessionStatePaused();
    drawSessionRun(app.session.stepDurationSec);
    return;
  }

  if (app.session.stepDurationSec <= 0)
    applyCurrentStepFromModel();
  if (app.session.stepTotalSec <= 0)
    app.session.stepTotalSec = app.session.stepDurationSec;

  setSessionStateRunning();
  app.session.stepStartMs = nowMs;
  drawSessionRun(app.session.stepDurationSec);
}
