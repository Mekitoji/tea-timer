#pragma once

struct AboutView {
  bool storageAvailable = false;
  unsigned long storageFreeKb = 0;
  unsigned long freeHeap = 0;
  unsigned long uptimeSeconds = 0;
  const char *firmwareVersion = "";
};
