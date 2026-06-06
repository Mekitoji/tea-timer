#pragma once

struct AudioSettingsView {
  bool enabled = false;
  const char *profile = "";
  bool enabledSelected = false;
  bool profileSelected = false;
  bool editMode = false;
};
