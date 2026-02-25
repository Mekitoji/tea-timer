#include "flow/timer_flow.h"

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <hw/audio.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
int lastRemaining = -1;
unsigned long timerHoldStartMs = 0;
bool timerWasDown = false;
bool timerLongPressFired = false;
unsigned long timerReleaseGuardUntilMs = 0;
} // namespace

int normalizeTimerPresetSec(int sec) {
  if (sec < MIN_TIME)
    return MIN_TIME;
  if (sec > MAX_TIME)
    return MAX_TIME;
  return sec;
}

void resetTimerButtonFlowState() {
  timerWasDown = false;
  timerLongPressFired = false;
  timerReleaseGuardUntilMs = 0;
}

void applyTimerPresetSec(int sec) {
  int v = normalizeTimerPresetSec(sec);
  timerDuration = v;
  editTimeValue = v;
  timerTotalSec = v;
}

void resetSingleTimerRuntimeState() {
  setTimerStateStopped();

  timerIgnoreReleaseAfterEnter = false;
  timerStartMillis = 0;
  lastRemaining = -1;
}

void timerLongResetToPreset() {
  applyTimerPresetSec(timerTotalSec);
  prefs.putInt(appcfg::PREFS_DURATION_KEY, timerDuration);
  resetSingleTimerRuntimeState();
  drawTimerScreen("Timer", editTimeValue, timerTotalSec);
}

void timerPauseAt(unsigned long nowMs) {
  unsigned long elapsed = (nowMs - timerStartMillis) / 1000;
  int remaining = timerDuration - (int)elapsed;
  if (remaining < 0)
    remaining = 0;

  setTimerStatePaused();
  timerDuration = remaining;
  editTimeValue = remaining;
  drawTimerScreen("Timer", remaining, timerTotalSec);
}

void timerStartOrResumeAt(unsigned long nowMs) {
  // Start from STOP preset
  if (isTimerStopped()) {
    if (editTimeValue < MIN_TIME)
      editTimeValue = MIN_TIME;
    if (editTimeValue > MAX_TIME)
      editTimeValue = MAX_TIME;

    timerTotalSec = editTimeValue;
    timerDuration = editTimeValue;
    drawTimerScreen("Timer", timerDuration, timerTotalSec);
  }

  if (timerDuration <= 0) {
    timerDuration = timerTotalSec;
  }

  setTimerStateRunning();
  timerStartMillis = nowMs;
  resetSingleTimerFlowState();
}

void updateSingleTimer() {
  if (currentScreen == SCREEN_TIMER) {
    if (!isTimerRunning())
      return;

    unsigned long elapsed = (millis() - timerStartMillis) / 1000;
    int remaining = timerDuration - (int)elapsed;
    if (remaining < 0)
      remaining = 0;

    if (remaining != lastRemaining) {
      drawTimerScreen("Timer", remaining, timerTotalSec);

      if (remaining <= 3 && remaining > 0) {
        digitalWrite(LED_PIN, HIGH);
        beep(2200, 60);
        digitalWrite(LED_PIN, LOW);
      }

      if (remaining == 0) {
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_PIN, HIGH);
          buzzerOn(2600);
          delay(80);
          buzzerOff();
          digitalWrite(LED_PIN, LOW);
          delay(120);
        }
        timerLongResetToPreset();
      }

      lastRemaining = remaining;
    }
  }
}

void timerAdjustByEncoderDelta(int delta) {
  if (isTimerRunning())
    return;

  if (isTimerStopped()) {
    editTimeValue += delta;
    if (editTimeValue < MIN_TIME)
      editTimeValue = MIN_TIME;
    if (editTimeValue > MAX_TIME)
      editTimeValue = MAX_TIME;

    timerTotalSec = editTimeValue;
    drawTimerScreen("Timer", editTimeValue, timerTotalSec);
    return;
  }

  int elapsed = timerTotalSec - timerDuration;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > MAX_TIME)
    elapsed = MAX_TIME;

  int maxRemaining = MAX_TIME - elapsed;
  if (maxRemaining < MIN_TIME)
    maxRemaining = MIN_TIME;

  int newRemaining = timerDuration + delta;
  if (newRemaining < MIN_TIME)
    newRemaining = MIN_TIME;
  if (newRemaining > maxRemaining)
    newRemaining = maxRemaining;

  timerDuration = newRemaining;
  editTimeValue = newRemaining;
  timerTotalSec = elapsed + newRemaining;
  drawTimerScreen("Timer", timerDuration, timerTotalSec);
}

void processTimerButtonInput(bool down, unsigned long nowMs) {
  // Ignore click that opened Timer from menu.
  if (timerIgnoreReleaseAfterEnter) {
    if (!down) {
      timerIgnoreReleaseAfterEnter = false;
      timerWasDown = false;
      timerLongPressFired = false;
    }
    return;
  }

  // Ignore bounce after long-press release.
  if (nowMs < timerReleaseGuardUntilMs) {
    if (!down)
      timerWasDown = false;
    return;
  }

  // Press edge
  if (down && !timerWasDown) {
    timerWasDown = true;
    timerHoldStartMs = nowMs;
    timerLongPressFired = false;
    return;
  }

  // Long press => reset
  if (down && timerWasDown && !timerLongPressFired &&
      (nowMs - timerHoldStartMs >= appcfg::TIMER_HOLD_MS)) {
    timerLongPressFired = true;
    timerReleaseGuardUntilMs = nowMs + appcfg::TIMER_RELEASE_GUARD_MS;
    timerLongResetToPreset();
    return;
  }

  // Release edge => short press
  if (!down && timerWasDown) {
    timerWasDown = false;

    if (timerLongPressFired) {
      timerLongPressFired = false;
      return;
    }

    if (isTimerRunning()) {
      timerPauseAt(nowMs);
    } else {
      timerStartOrResumeAt(nowMs);
    }
  }
}

void resetSingleTimerFlowState() { lastRemaining = -1; }
