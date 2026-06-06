#pragma once

inline constexpr int SESSION_HISTORY_VIEW_MAX_ITEMS = 128;

struct SessionHistoryItemView {
  const char *title = "";
  int completedInfusions = 0;
  bool finishedEarly = false;
};

struct SessionHistoryDetailView {
  const char *title = "";
  const char *startedAt = "";
  const char *finishedAt = "";
  int completedInfusions = 0;
  int rinseSec = 0;
  bool finishedEarly = false;
};

struct SessionHistoryView {
  const SessionHistoryItemView *items = nullptr;
  int itemCount = 0;
  int selectedIndex = 0;
  bool detailOpen = false;
  SessionHistoryDetailView detail;
  bool deleteConfirmActive = false;
  bool deleteConfirmYesSelected = false;
};
