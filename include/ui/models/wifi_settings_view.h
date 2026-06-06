#pragma once

struct WifiSettingsView {
  const char *badge = "";
  const char *row1 = "";
  const char *row2 = "";
  const char *row3 = "";
  const char *row4 = "";
  const char *footer = "";
  bool resetConfirmActive = false;
  bool resetConfirmYesSelected = false;
};
