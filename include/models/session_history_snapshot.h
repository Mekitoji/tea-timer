#pragma once

struct SessionLogRecord;

struct SessionHistorySnapshot {
  const SessionLogRecord *records = nullptr;
  int recordCount = 0;
  int selectedIndex = 0;
  bool detailOpen = false;
  bool deleteConfirmActive = false;
  bool deleteConfirmYesSelected = false;
};
