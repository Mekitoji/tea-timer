#pragma once

struct ClockSettingsView {
  const char *badge = "";
  const char *time = "";
  const char *date = "";
  const char *autoSync = "";
  bool timeSelected = false;
  bool dateSelected = false;
  bool autoSyncSelected = false;
  bool editMode = false;
};
