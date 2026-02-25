#include "flow/session_flow.h"

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/menu_flow.h>
#include <hw/audio.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
int lastRemaining = -1;
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

  sessionStepIndex++;

  if (sessionStepIndex >= SESSION_STEP_COUNT) {
    setSessionStateCompleted();
    drawSessionComplete();
    return;
  }

  sessionStepDurationSec = SESSION_STEPS[sessionStepIndex];
  sessionStepTotalSec = sessionStepDurationSec;
  setSessionStatePaused();
  drawSessionRun(sessionStepDurationSec);
}

void updateSessionRun() {
  if (currentScreen == SCREEN_SESSION_RUN) {
    if (sessionStepIndex >= SESSION_STEP_COUNT) {
      if (!isSessionCompleted()) {
        setSessionStateCompleted();
        drawSessionComplete();
      }
      return;
    }

    int stepSec = sessionStepDurationSec > 0 ? sessionStepDurationSec
                                             : SESSION_STEPS[sessionStepIndex];
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
        sessionStepIndex++;

        if (sessionStepIndex >= SESSION_STEP_COUNT) {
          for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            buzzerOn(3000);
            delay(120);
            buzzerOff();
            digitalWrite(LED_PIN, LOW);
            delay(160);
          }

          setSessionStateCompleted();
          drawSessionComplete();
          lastRemaining = -999;
          return;
        }

        sessionStepDurationSec = SESSION_STEPS[sessionStepIndex];
        sessionStepTotalSec = sessionStepDurationSec;
        drawSessionRun(sessionStepDurationSec);
      }

      lastRemaining = remaining;
    }
  }
}

void sessionToggleRunPauseAt(unsigned long nowMs) {
  if (sessionStepIndex >= SESSION_STEP_COUNT) {
    goToMenu();
    return;
  }

  if (isSessionRunning()) {
    int stepSec = sessionStepDurationSec > 0 ? sessionStepDurationSec
                                             : SESSION_STEPS[sessionStepIndex];
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
    sessionStepDurationSec = SESSION_STEPS[sessionStepIndex];
  if (sessionStepTotalSec <= 0)
    sessionStepTotalSec = sessionStepDurationSec;

  setSessionStateRunning();
  sessionStepStartMs = nowMs;
  drawSessionRun(sessionStepDurationSec);
}

void sessionStopAndExitToMenu() {
  setSessionStateStopped();
  goToMenu();
}
