#pragma once

struct SessionPresetView {
  bool hasPreset = false;
  int presetIndex = 0;
  int presetCount = 0;
  const char *name = "";
  const char *dosePer100ml = "";
  const char *tempC = "";
  int infusionCount = 0;
};

struct SessionCompleteView {
  const char *teaName = "";
};

struct SessionRunView {
  const char *status = "";
  const char *teaName = "";
  bool rinseActive = false;
  int stepIndex = 0;
  int infusionCount = 0;
  int totalSec = 0;
  int previousSec = 0;
  int remaining = 0;
  bool running = false;
  bool endConfirmActive = false;
  bool endConfirmYesSelected = false;
};
