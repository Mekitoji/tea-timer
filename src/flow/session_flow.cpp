#include <flow/session_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/clock_time.h>
#include <app/long_press.h>
#include <app/session_presets.h>
#include <app/tea_config.h>
#include <flow/audio_profile_flow.h>
#include <flow/navigation_flow.h>
#include <flow/session_journal_flow.h>
#include <flow/session_runtime_snapshot_flow.h>
#include <hw/feedback.h>
#include <ui.h>

namespace {
int lastRemaining = -1;
int lastPersistedSnapshotRemaining = -1;

int stepSecAt(int index) {
  if (index < 0 || index >= app.session.stepCount)
    return MIN_TIME;
  return clampTeaDurationSec(app.session.steps[index]);
}

bool hasCurrentSessionStep() {
  if (app.session.rinseActive)
    return true;
  return app.session.stepIndex >= 0 &&
         app.session.stepIndex < app.session.stepCount;
}

int currentStepSecFromState() {
  int stepSec = app.session.stepDurationSec;
  if (stepSec <= 0) {
    stepSec = app.session.rinseActive ? app.session.rinseSec
                                      : stepSecAt(app.session.stepIndex);
  }
  if (app.session.rinseActive)
    return clampOptionalTeaDurationSec(stepSec);
  return clampTeaDurationSec(stepSec);
}

void applyCurrentStepFromModel() {
  if (app.session.rinseActive) {
    int rinse = clampOptionalTeaDurationSec(app.session.rinseSec);
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

LongPressTracker sessionLongPress;

void ensureSessionStarted() {
  if (app.session.started)
    return;

  app.session.started = true;
  app.session.startedAt = clockCurrentEpochOrZero();
}

void markSessionSnapshotPersisted(int remaining) {
  lastPersistedSnapshotRemaining = remaining;
}

void persistRunningSessionSnapshotIfNeeded(unsigned long nowMs, int remaining) {
  if (!isSessionRunning() || !app.session.started)
    return;

  if (lastPersistedSnapshotRemaining >= 0 &&
      remaining <= lastPersistedSnapshotRemaining &&
      lastPersistedSnapshotRemaining - remaining <
          appcfg::SESSION_RUNTIME_SNAPSHOT_INTERVAL_SEC) {
    return;
  }

  persistSessionRuntimeSnapshot(nowMs);
  markSessionSnapshotPersisted(remaining);
}
} // namespace

void resetSessionFlowState() {
  lastRemaining = -1;
  lastPersistedSnapshotRemaining = -1;
}

void resetSessionLongPressFlowState() {
  sessionLongPress.reset();
}

void processSessionLongPressInput(bool down, unsigned long nowMs) {
  if (sessionLongPress.update(down, nowMs, appcfg::SESSION_HOLD_MS) !=
      LongPressEvent::LongPressed)
    return;

  if (!app.session.rinseActive &&
      app.session.stepIndex >= app.session.stepCount - 1) {
    persistCompletedSessionJournalRecord(true);
    app.session.stepIndex = app.session.stepCount;
    setSessionStateCompleted();
    clearSessionRuntimeSnapshot();
    drawSessionComplete();
    return;
  }

  if (!advanceToNextSessionStep()) {
    persistCompletedSessionJournalRecord(true);
    setSessionStateCompleted();
    clearSessionRuntimeSnapshot();
    drawSessionComplete();
    return;
  }

  setSessionStatePaused();
  persistSessionRuntimeSnapshot(nowMs);
  markSessionSnapshotPersisted(app.session.stepDurationSec);
  drawSessionRun(app.session.stepDurationSec);
}

void loadSessionPresetByIndex(int presetIndex) {
  app.session.endConfirm.active = false;
  app.session.endConfirm.yesSelected = false;

  if (SESSION_PRESET_COUNT <= 0) {
    app.session.presetIndex = 0;
    app.session.started = false;
    app.session.startedAt = 0;
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
  app.session.started = false;
  app.session.startedAt = 0;

  const SessionPreset &preset = SESSION_PRESETS[app.session.presetIndex];
  app.session.rinseSec = clampOptionalTeaDurationSec(preset.rinseSec);
  app.session.rinseActive = true;

  int count = preset.stepCount;
  if (count < 0)
    count = 0;
  if (count > SESSION_MAX_STEPS)
    count = SESSION_MAX_STEPS;

  app.session.stepCount = count;

  for (int i = 0; i < app.session.stepCount; i++) {
    app.session.steps[i] = clampTeaDurationSec(preset.stepsSec[i]);
  }
  for (int i = app.session.stepCount; i < SESSION_MAX_STEPS; i++) {
    app.session.steps[i] = 0;
  }

  app.session.stepIndex = 0;
  app.session.stepDurationSec = app.session.rinseSec;
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

  if (app.session.stepCount <= 0)
    return;

  app.session.stepIndex = 0;
  app.session.rinseActive = true;
  app.session.started = false;
  app.session.startedAt = 0;
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
        clearSessionRuntimeSnapshot();
        drawSessionComplete();
      }
      return;
    }

    int stepSec = currentStepSecFromState();
    int remaining = stepSec;

    if (isSessionRunning()) {
      unsigned long elapsed = (millis() - app.session.stepStartMs) / 1000;
      remaining = stepSec - (int)elapsed;
      if (remaining < 0)
        remaining = 0;
    }

    if (remaining != lastRemaining) {
      drawSessionRun(remaining);
      persistRunningSessionSnapshotIfNeeded(millis(), remaining);

      if (isSessionRunning() && remaining <= 3 && remaining > 0) {
        pulseLedAndAudio(audioProfileCountdownFreq(),
                         audioProfileBeepDurationMs(),
                         app.audio.audioEnabled);
      }
    }

    if (isSessionRunning() && remaining == 0) {
      for (int i = 0; i < 2; i++) {
        pulseLedAndAudio(audioProfileSessionStepDoneFreq(),
                         audioProfileBeepDurationMs(),
                         app.audio.audioEnabled);
        delay(120);
      }

      setSessionStatePaused();
      if (!advanceToNextSessionStep()) {
        for (int i = 0; i < 3; i++) {
          pulseLedAndAudio(audioProfileSessionDoneFreq(),
                           audioProfileBeepDurationMs(),
                           app.audio.audioEnabled);
          delay(160);
        }

        setSessionStateCompleted();
        app.session.endConfirm.active = false;
        app.session.endConfirm.yesSelected = false;
        persistCompletedSessionJournalRecord(false);
        clearSessionRuntimeSnapshot();
        drawSessionComplete();
        lastRemaining = -999;
        return;
      }

      drawSessionRun(app.session.stepDurationSec);
      persistSessionRuntimeSnapshot(millis());
      markSessionSnapshotPersisted(app.session.stepDurationSec);
      lastRemaining = -1;
      return;
    }

    lastRemaining = remaining;
  }
}

void sessionToggleRunPauseAt(unsigned long nowMs) {
  if (!hasCurrentSessionStep()) {
    clearSessionRuntimeSnapshot();
    navigateTo(SCREEN_MENU);
    drawMenu();

    return;
  }

  if (isSessionRunning()) {
    int stepSec = currentStepSecFromState();

    unsigned long elapsed = (nowMs - app.session.stepStartMs) / 1000;
    int remaining = stepSec - (int)elapsed;
    if (remaining < 0)
      remaining = 0;

    app.session.stepDurationSec = remaining;
    setSessionStatePaused();
    persistSessionRuntimeSnapshot(nowMs);
    markSessionSnapshotPersisted(app.session.stepDurationSec);
    drawSessionRun(app.session.stepDurationSec);
    return;
  }

  if (app.session.stepDurationSec <= 0)
    applyCurrentStepFromModel();
  if (app.session.stepTotalSec <= 0)
    app.session.stepTotalSec = app.session.stepDurationSec;

  if (app.session.rinseActive && app.session.stepDurationSec <= 0) {
    if (!advanceToNextSessionStep()) {
      setSessionStateCompleted();
      clearSessionRuntimeSnapshot();
      drawSessionComplete();
      return;
    }

    setSessionStatePaused();
    drawSessionRun(app.session.stepDurationSec);
    return;
  }

  ensureSessionStarted();
  setSessionStateRunning();
  app.session.stepStartMs = nowMs;
  persistSessionRuntimeSnapshot(nowMs);
  markSessionSnapshotPersisted(app.session.stepDurationSec);
  drawSessionRun(app.session.stepDurationSec);
}

void sessionAdjustPausedStepByDelta(int delta) {
  if (isSessionRunning() || !hasCurrentSessionStep())
    return;

  int remaining = currentStepSecFromState();

  int elapsed = app.session.stepTotalSec - remaining;
  if (elapsed < 0)
    elapsed = 0;
  if (elapsed > MAX_TIME)
    elapsed = MAX_TIME;

  int minRemaining = app.session.rinseActive ? 0 : MIN_TIME;
  int maxRemaining = MAX_TIME - elapsed;
  if (maxRemaining < minRemaining)
    maxRemaining = minRemaining;

  int newRemaining = remaining + delta;
  if (newRemaining < minRemaining)
    newRemaining = minRemaining;
  if (newRemaining > MAX_TIME)
    newRemaining = MAX_TIME;
  if (newRemaining > maxRemaining)
    newRemaining = maxRemaining;

  app.session.stepDurationSec = newRemaining;
  app.session.stepTotalSec = elapsed + newRemaining;

  if (app.session.rinseActive) {
    app.session.rinseSec = app.session.stepTotalSec;
  } else if (app.session.stepIndex >= 0 &&
             app.session.stepIndex < app.session.stepCount) {
    app.session.steps[app.session.stepIndex] = app.session.stepTotalSec;
  }

  persistSessionRuntimeSnapshot(millis());
  markSessionSnapshotPersisted(app.session.stepDurationSec);
}
