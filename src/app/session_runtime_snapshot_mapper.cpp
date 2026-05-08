#include <app/session_runtime_snapshot_mapper.h>

#include <app/session_presets.h>
#include <app/tea_config.h>

namespace {
bool hasRestorableStep(const SessionRuntimeSnapshot &snapshot) {
  if (snapshot.rinseActive && snapshot.rinseSec > 0)
    return true;

  return snapshot.stepIndex >= 0 && snapshot.stepIndex < snapshot.stepCount;
}
} // namespace

SessionRuntimeSnapshot
buildSessionRuntimeSnapshot(const SessionStateModel &session,
                            SessionState state, unsigned long remainingSec) {
  SessionRuntimeSnapshot snapshot{
      .valid = session.started && state != SessionState::Stopped &&
               state != SessionState::Completed,
      .presetIndex = session.presetIndex,
      .sessionStarted = session.started,
      .startedAt = session.startedAt,
      .rinseActive = session.rinseActive,
      .rinseSec = session.rinseSec,
      .stepIndex = session.stepIndex,
      .stepCount = session.stepCount,
      .stepDurationSec = session.stepDurationSec,
      .stepTotalSec = session.stepTotalSec,
      .remainingSec = remainingSec,
      .state = state,
  };

  if (snapshot.stepCount < 0)
    snapshot.stepCount = 0;
  if (snapshot.stepCount > SESSION_MAX_STEPS)
    snapshot.stepCount = SESSION_MAX_STEPS;

  for (int i = 0; i < snapshot.stepCount; i++) {
    snapshot.stepsSec[i] = session.steps[i];
  }

  return snapshot;
}

bool applySessionRuntimeSnapshot(const SessionRuntimeSnapshot &snapshot,
                                 SessionStateModel &session) {
  if (!snapshot.valid || !snapshot.sessionStarted)
    return false;
  if (snapshot.state == SessionState::Stopped ||
      snapshot.state == SessionState::Completed)
    return false;
  if (SESSION_PRESET_COUNT <= 0 || snapshot.presetIndex < 0 ||
      snapshot.presetIndex >= SESSION_PRESET_COUNT)
    return false;
  if (snapshot.stepCount < 0 || snapshot.stepCount > SESSION_MAX_STEPS)
    return false;
  if (!hasRestorableStep(snapshot))
    return false;

  session.endConfirm.active = false;
  session.endConfirm.yesSelected = false;
  session.presetIndex = snapshot.presetIndex;
  session.started = true;
  session.startedAt = snapshot.startedAt;
  session.rinseActive = snapshot.rinseActive;
  session.rinseSec = snapshot.rinseSec;
  session.stepIndex = snapshot.stepIndex;
  session.stepCount = snapshot.stepCount;

  for (int i = 0; i < SESSION_MAX_STEPS; i++) {
    session.steps[i] = i < snapshot.stepCount ? snapshot.stepsSec[i] : 0;
  }

  session.stepDurationSec =
      clampTeaDurationSec(static_cast<int>(snapshot.remainingSec));

  session.stepTotalSec = snapshot.stepTotalSec;
  if (session.stepTotalSec < session.stepDurationSec)
    session.stepTotalSec = session.stepDurationSec;
  session.stepTotalSec = clampTeaDurationSec(session.stepTotalSec);

  session.stepStartMs = 0;
  return true;
}
