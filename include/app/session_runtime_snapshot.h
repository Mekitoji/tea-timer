#pragma once

#include <cstdint>

#include <app/app_config.h>
#include <app/session_presets.h>
#include <app/session_state.h>

/* SessionRuntimeSnapshot represents the current state of a session in progress
 Used for restore session if device turn off or restart during active session
 After session is completed, the snapshot is cleared and entry of
 SessionLogRecord created */
struct SessionRuntimeSnapshot {
  uint8_t version = appcfg::SESSION_RUNTIME_SNAPSHOT_VERSION;
  bool valid = false;

  int presetIndex = 0;
  bool sessionStarted = false;
  unsigned long startedAt = 0;

  bool rinseActive = false;
  int rinseSec = 0;

  int stepIndex = 0;
  int stepCount = 0;
  int stepsSec[SESSION_MAX_STEPS] = {0};

  int stepDurationSec = 0;
  int stepTotalSec = 0;
  unsigned long remainingSec = 0;

  SessionState state = SessionState::Stopped;
};
