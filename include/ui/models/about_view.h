#pragma once

struct AboutView {
  const char *chip = "";
  bool storageAvailable = false;
  unsigned long storageFreeKb = 0;
  unsigned long freeHeap = 0;
  const char *firmwareVersion = "";
};
