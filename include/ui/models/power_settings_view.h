#pragma once

struct PowerSettingsView {
  bool enabled = false;
  const char *timeout = "";
  bool enabledSelected = false;
  bool timeoutSelected = false;
  bool editMode = false;
};
