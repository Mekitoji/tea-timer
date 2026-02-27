#pragma once

constexpr int SESSION_MAX_STEPS = 20;

struct SessionPreset {
  const char *name;
  const char *dosePer100ml;
  const char *tempC;
  int rinseSec;
  const int *stepsSec;
  int stepCount;
};

extern const SessionPreset SESSION_PRESETS[];
extern const int SESSION_PRESET_COUNT;
