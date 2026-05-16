#pragma once

#include <app/session_presets.h>

// SessionLogRecord represents a completed session
struct SessionLogRecord {
  char id[37] = {}; // uuid

  int presetIndex = 0;
  char presetName[32] = {};

  unsigned long startedAt = 0;
  unsigned long finishedAt = 0;
  bool finishedEarly = false;

  int completedInfusionCount = 0;
  int infusionSec[SESSION_MAX_STEPS] = {0};
  unsigned long infusionStartedAt[SESSION_MAX_STEPS] = {0};

  int rinseSec = 0;
  unsigned long rinseStartedAt = 0;
};
