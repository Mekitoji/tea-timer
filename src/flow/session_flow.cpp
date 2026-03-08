#include <flow/session_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/session_presets.h>
#include <app/tea_config.h>
#include <flow/navigation_flow.h>
#include <hw/audio.h>
#include <hw/pins.h>
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
  if (index < 0 || index >= sessionStepCount)
    return MIN_TIME;
  return clampInfusionSec(sessionSteps[index]);
}

bool hasCurrentSessionStep() {
  if (sessionRinseActive && sessionRinseSec > 0)
    return true;
  return sessionStepIndex >= 0 && sessionStepIndex < sessionStepCount;
}

void applyCurrentStepFromModel() {
  if (sessionRinseActive) {
    int rinse = sessionRinseSec;
    if (rinse < MIN_TIME)
      rinse = MIN_TIME;
    if (rinse > MAX_TIME)
      rinse = MAX_TIME;
    sessionRinseSec = rinse;
    sessionStepDurationSec = rinse;
    sessionStepTotalSec = rinse;
    return;
  }

  sessionStepDurationSec = stepSecAt(sessionStepIndex);
  sessionStepTotalSec = sessionStepDurationSec;
}

bool advanceToNextSessionStep() {
  if (sessionRinseActive) {
    sessionRinseActive = false;
    sessionStepIndex = 0;
    if (sessionStepCount <= 0)
      return false;
    applyCurrentStepFromModel();
    return true;
  }

  sessionStepIndex++;
  if (sessionStepIndex >= sessionStepCount)
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
  drawSessionRun(sessionStepDurationSec);
}

void loadSessionPresetByIndex(int presetIndex) {
  sessionEndConfirmActive = false;
  sessionEndConfirmYes = false;

  if (SESSION_PRESET_COUNT <= 0) {
    sessionPresetIndex = 0;
    sessionStepCount = 0;
    sessionRinseSec = 0;
    sessionRinseActive = false;
    setSessionStateStopped();
    resetSessionFlowState();
    return;
  }

  if (presetIndex < 0)
    presetIndex = SESSION_PRESET_COUNT - 1;
  if (presetIndex >= SESSION_PRESET_COUNT)
    presetIndex = 0;

  sessionPresetIndex = presetIndex;

  const SessionPreset &preset = SESSION_PRESETS[sessionPresetIndex];
  sessionRinseSec = clampRinseSec(preset.rinseSec);
  sessionRinseActive = (sessionRinseSec > 0);

  int count = preset.stepCount;
  if (count < 0)
    count = 0;
  if (count > SESSION_MAX_STEPS)
    count = SESSION_MAX_STEPS;

  sessionStepCount = count;

  for (int i = 0; i < sessionStepCount; i++) {
    sessionSteps[i] = clampInfusionSec(preset.stepsSec[i]);
  }
  for (int i = sessionStepCount; i < SESSION_MAX_STEPS; i++) {
    sessionSteps[i] = 0;
  }

  sessionStepIndex = 0;
  if (sessionRinseActive) {
    sessionStepDurationSec = sessionRinseSec;
  } else if (sessionStepCount > 0) {
    sessionStepDurationSec = stepSecAt(0);
  } else {
    sessionStepDurationSec = MIN_TIME;
  }
  sessionStepTotalSec = sessionStepDurationSec;

  setSessionStateStopped();
  resetSessionFlowState();
}

void enterSessionRunFromCurrentPreset() {
  sessionEndConfirmActive = false;
  sessionEndConfirmYes = false;

  if (sessionStepCount <= 0) {
    loadSessionPresetByIndex(sessionPresetIndex);
  }

  if (sessionStepCount <= 0 && sessionRinseSec <= 0)
    return;

  sessionStepIndex = 0;
  sessionRinseActive = (sessionRinseSec > 0);
  applyCurrentStepFromModel();

  setSessionStatePaused();
  navigateTo(SCREEN_SESSION_RUN);
  drawSessionRun(sessionStepDurationSec);
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

    int stepSec = sessionStepDurationSec;
    if (stepSec <= 0) {
      stepSec =
          sessionRinseActive ? sessionRinseSec : stepSecAt(sessionStepIndex);
    }
    if (stepSec < MIN_TIME)
      stepSec = MIN_TIME;
    if (stepSec > MAX_TIME)
      stepSec = MAX_TIME;
    int remaining = stepSec;

    if (isSessionRunning()) {
      unsigned long elapsed = (millis() - sessionStepStartMs) / 1000;
      remaining = stepSec - (int)elapsed;
      if (remaining < 0)
        remaining = 0;
    }

    if (remaining != lastRemaining) {
      drawSessionRun(remaining);

      if (isSessionRunning() && remaining <= 3 && remaining > 0) {
        digitalWrite(LED_PIN, HIGH);
        beep(2200, 60);
        digitalWrite(LED_PIN, LOW);
      }
    }

    if (isSessionRunning() && remaining == 0) {
      for (int i = 0; i < 2; i++) {
        digitalWrite(LED_PIN, HIGH);
        buzzerOn(2500);
        delay(70);
        buzzerOff();
        digitalWrite(LED_PIN, LOW);
        delay(120);
      }

      setSessionStatePaused();
      if (!advanceToNextSessionStep()) {
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          buzzerOn(3000);
          delay(120);
          buzzerOff();
          digitalWrite(LED_PIN, LOW);
          delay(160);
        }

        setSessionStateCompleted();
        sessionEndConfirmActive = false;
        sessionEndConfirmYes = false;
        drawSessionComplete();
        lastRemaining = -999;
        return;
      }

      drawSessionRun(sessionStepDurationSec);
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
    int stepSec = sessionStepDurationSec;
    if (stepSec <= 0) {
      stepSec =
          sessionRinseActive ? sessionRinseSec : stepSecAt(sessionStepIndex);
    }
    if (stepSec < MIN_TIME)
      stepSec = MIN_TIME;
    if (stepSec > MAX_TIME)
      stepSec = MAX_TIME;

    unsigned long elapsed = (nowMs - sessionStepStartMs) / 1000;
    int remaining = stepSec - (int)elapsed;
    if (remaining < 0)
      remaining = 0;

    sessionStepDurationSec = remaining;
    setSessionStatePaused();
    drawSessionRun(sessionStepDurationSec);
    return;
  }

  if (sessionStepDurationSec <= 0)
    applyCurrentStepFromModel();
  if (sessionStepTotalSec <= 0)
    sessionStepTotalSec = sessionStepDurationSec;

  setSessionStateRunning();
  sessionStepStartMs = nowMs;
  drawSessionRun(sessionStepDurationSec);
}
