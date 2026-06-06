#include <flow/timer_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/long_press.h>
#include <app/tea_config.h>
#include <flow/audio_profile_flow.h>
#include <hw/feedback.h>
#include <presentation/timer_presenter.h>
#include <storage/settings_store.h>

namespace {
int lastRemaining = -1;
LongPressTracker timerLongPress;
} // namespace

int normalizeTimerPresetSec(int sec) { return clampTeaDurationSec(sec); }

void resetTimerLongPressFlowState() {
  timerLongPress.reset();
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
  timerRender(app.timer.editTimeValue);
}

void timerPauseAt(unsigned long nowMs) {
  unsigned long elapsed = (nowMs - app.timer.timerStartMillis) / 1000;
  int remaining = app.timer.timerDuration - (int)elapsed;
  if (remaining < 0)
    remaining = 0;

  setTimerStatePaused();
  app.timer.timerDuration = remaining;
  app.timer.editTimeValue = remaining;
  timerRender(remaining);
}

void timerStartOrResumeAt(unsigned long nowMs) {
  // Start from STOP preset
  if (isTimerStopped()) {
    app.timer.editTimeValue = clampTeaDurationSec(app.timer.editTimeValue);

    app.timer.timerTotalSec = app.timer.editTimeValue;
    app.timer.timerDuration = app.timer.editTimeValue;
    timerRender(app.timer.timerDuration);
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
      timerRender(remaining);

      if (remaining <= 3 && remaining > 0) {
        pulseLedAndAudio(audioProfileCountdownFreq(),
                         audioProfileBeepDurationMs(),
                         app.audio.audioEnabled);
      }

      if (remaining == 0) {
        for (int i = 0; i < 3; i++) {
          pulseLedAndAudio(audioProfileTimerDoneFreq(),
                           audioProfileBeepDurationMs(),
                           app.audio.audioEnabled);
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
    app.timer.editTimeValue =
        clampTeaDurationSec(app.timer.editTimeValue + delta);

    app.timer.timerTotalSec = app.timer.editTimeValue;
    timerRender(app.timer.editTimeValue);
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

  int newRemaining = clampTeaDurationSec(app.timer.timerDuration + delta);
  if (newRemaining > maxRemaining)
    newRemaining = maxRemaining;

  app.timer.timerDuration = newRemaining;
  app.timer.editTimeValue = newRemaining;
  app.timer.timerTotalSec = elapsed + newRemaining;
  timerRender(app.timer.timerDuration);
}

void processTimerLongPressInput(bool down, unsigned long nowMs) {
  if (timerLongPress.update(down, nowMs, appcfg::TIMER_HOLD_MS) ==
      LongPressEvent::LongPressed) {
    timerLongResetToPreset();
  }
}

void resetSingleTimerFlowState() { lastRemaining = -1; }
