#pragma once

struct TimerView {
  const char *title = "";
  const char *status = "";
  int secondsLeft = 0;
  int totalSeconds = 0;
  bool showProgress = false;
};
