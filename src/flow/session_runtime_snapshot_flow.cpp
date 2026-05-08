#include <flow/session_runtime_snapshot_flow.h>

#include <Arduino.h>

#include <app/app_state.h>
#include <app/session_runtime_snapshot_mapper.h>
#include <app/tea_config.h>
#include <flow/navigation_flow.h>
#include <flow/session_flow.h>
#include <storage/session_runtime_store.h>
#include <ui.h>

namespace {
unsigned long remainingSecForSnapshot(unsigned long nowMs) {
  int remaining = app.session.stepDurationSec;

  if (isSessionRunning()) {
    unsigned long elapsed = (nowMs - app.session.stepStartMs) / 1000;
    remaining -= static_cast<int>(elapsed);
  }

  if (remaining < 0)
    remaining = 0;
  if (remaining > MAX_TIME)
    remaining = MAX_TIME;

  return static_cast<unsigned long>(remaining);
}
} // namespace

bool restoreSessionRuntimeSnapshotOnBoot() {
  SessionRuntimeSnapshot snapshot;
  if (!sessionRuntimeStoreLoad(snapshot))
    return false;

  if (!applySessionRuntimeSnapshot(snapshot, app.session)) {
    sessionRuntimeStoreClear();
    return false;
  }

  setSessionStatePaused();
  resetSessionFlowState();
  resetSessionLongPressFlowState();
  navigateTo(SCREEN_SESSION_RUN);
  drawSessionRun(app.session.stepDurationSec);
  return true;
}

void persistSessionRuntimeSnapshot(unsigned long nowMs) {
  if (!app.session.started || isSessionStopped() || isSessionCompleted()) {
    sessionRuntimeStoreClear();
    return;
  }

  unsigned long remainingSec = remainingSecForSnapshot(nowMs);
  SessionRuntimeSnapshot snapshot =
      buildSessionRuntimeSnapshot(app.session, sessionState, remainingSec);
  if (!snapshot.valid) {
    sessionRuntimeStoreClear();
    return;
  }

  sessionRuntimeStoreSave(snapshot);
}

void clearSessionRuntimeSnapshot() { sessionRuntimeStoreClear(); }
