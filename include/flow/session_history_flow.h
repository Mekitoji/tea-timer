#pragma once

#include <app/session_log.h>

struct SessionHistorySnapshot {
  const SessionLogRecord *records = nullptr;
  int recordCount = 0;
  int selectedIndex = 0;
  bool detailOpen = false;
  bool deleteConfirmActive = false;
  bool deleteConfirmYesSelected = false;
};

void sessionHistoryEnter();
SessionHistorySnapshot sessionHistorySnapshot();
void sessionHistoryHandleEncoder(bool stepPlus, bool stepMinus);
void sessionHistoryHandleSelect();
void sessionHistoryHandleBack();
void sessionHistoryHandleLongPress(bool down, unsigned long nowMs);
void resetSessionHistoryLongPressState();
