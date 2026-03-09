#include <flow/timer_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <flow/audio_profile_flow.h>
#include <hw/feedback.h>
#include <storage/settings_store.h>
#include <ui.h>

namespace {
int lastRemaining = -1;
unsigned long timerHoldStartMs = 0;
bool timerWasDown = false;
bool timerLongPressFired = false;
} // namespace

int normalizeTimerPresetSec(int sec) {
  if (sec < MIN_TIME)
    return MIN_TIME;
  if (sec > MAX_TIME)
    return MAX_TIME;
  return sec;
}

void resetTimerLongPressFlowState() {
  timerWasDown = false;
  timerLongPressFired = false;
}

void applyTimerPresetSec(int sec) {
  int v = normalizeTimerPresetSec(sec);
  app.timer.timerDuration = v;
  app.timer.editTimeValue = v;
  app.timer.timerTotalSec = v;
}

void resetSingleTimerRuntimeState() {
  setTimerStateStopped();

  app.timer.timerIgnoreReleaseAfterEnter = false;
  app.timer.timerStartMillis = 0;
  lastRemaining = -1;
}

void timerLongResetToPreset() {
  applyTimerPresetSec(app.timer.timerTotalSec);
  settingsStoreSaveTimerDurationSec(app.timer.timerDuration);
  resetSingleTimerRuntimeState();
  drawTimerScreen("Timer", app.timer.editTimeValue, app.timer.timerTotalSec);
}

void timerPauseAt(unsigned long nowMs) {
  unsigned long elapsed = (nowMs - app.timer.timerStartMillis) / 1000;
  int remaining = app.timer.timerDuration - (int)elapsed;
  if (remaining < 0)
    remaining = 0;

  setTimerStatePaused();
  app.timer.timerDuration = remaining;
  app.timer.editTimeValue = remaining;
  drawTimerScreen("Timer", remaining, app.timer.timerTotalSec);
}

void timerStartOrResumeAt(unsigned long nowMs) {
  // Start from STOP preset
  if (isTimerStopped()) {
    if (app.timer.editTimeValue < MIN_TIME)
      app.timer.editTimeValue = MIN_TIME;
    if (app.timer.editTimeValue > MAX_TIME)
      app.timer.editTimeValue = MAX_TIME;

    app.timer.timerTotalSec = app.timer.editTimeValue;
    app.timer.timerDuration = app.timer.editTimeValue;
    drawTimerScreen("Timer", app.timer.timerDuration, app.timer.timerTotalSec);
  }

  if (app.timer.timerDuration <= 0) {
    app.timer.timerDuration = app.timer.timerTotalSec;
  }

  setTimerStateRunning();
  app.timer.timerStartMillis = nowMs;
  resetSingleTimerFlowState();
}

void updateSingleTimer() {
  if (currentScreen == SCREEN_TIMER) {
    if (!isTimerRunning())
      return;

    unsigned long elapsed = (millis() - app.timer.timerStartMillis) / 1000;
    int remaining = app.timer.timerDuration - (int)elapsed;
    if (remaining < 0)
      remaining = 0;

    if (remaining != lastRemaining) {
      drawTimerScreen("Timer", remaining, app.timer.timerTotalSec);

      if (remaining <= 3 && remaining > 0) {
        pulseLedAndSound(audioProfileCountdownFreq(),
                         audioProfileBeepDurationMs(),
                         app.audio.soundEnabled);
      }

      if (remaining == 0) {
        for (int i = 0; i < 3; i++) {
          pulseLedAndSound(audioProfileTimerDoneFreq(),
                           audioProfileBeepDurationMs(),
                           app.audio.soundEnabled);
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
    app.timer.editTimeValue += delta;
    if (app.timer.editTimeValue < MIN_TIME)
      app.timer.editTimeValue = MIN_TIME;
    if (app.timer.editTimeValue > MAX_TIME)
      app.timer.editTimeValue = MAX_TIME;

    app.timer.timerTotalSec = app.timer.editTimeValue;
    drawTimerScreen("Timer", app.timer.editTimeValue, app.timer.timerTotalSec);
    return;
  }

  int elapsed = app.timer.timerTotalSec - app.timer.timerDuration;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > MAX_TIME)
    elapsed = MAX_TIME;

  int maxRemaining = MAX_TIME - elapsed;
  if (maxRemaining < MIN_TIME)
    maxRemaining = MIN_TIME;

  int newRemaining = app.timer.timerDuration + delta;
  if (newRemaining < MIN_TIME)
    newRemaining = MIN_TIME;
  if (newRemaining > maxRemaining)
    newRemaining = maxRemaining;

  app.timer.timerDuration = newRemaining;
  app.timer.editTimeValue = newRemaining;
  app.timer.timerTotalSec = elapsed + newRemaining;
  drawTimerScreen("Timer", app.timer.timerDuration, app.timer.timerTotalSec);
}

void processTimerLongPressInput(bool down, unsigned long nowMs) {
  if (down && !timerWasDown) {
    timerWasDown = true;
    timerHoldStartMs = nowMs;
    timerLongPressFired = false;
    return;
  }

  if (!down && timerWasDown) {
    timerWasDown = false;
    timerLongPressFired = false;
    return;
  }

  // Long press => reset
  if (down && timerWasDown && !timerLongPressFired &&
      (nowMs - timerHoldStartMs >= appcfg::TIMER_HOLD_MS)) {
    timerLongPressFired = true;
    timerLongResetToPreset();
    return;
  }
}

void resetSingleTimerFlowState() { lastRemaining = -1; }
