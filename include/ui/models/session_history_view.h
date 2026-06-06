#pragma once

// Maximum number of entries exposed by SessionHistoryView. The value matches
// the current session journal capacity and may be configured alongside it.
// Invariant: producers must ensure itemCount <= SESSION_HISTORY_VIEW_MAX_ITEMS
// before exposing items; entries beyond the bound must be truncated.
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
  // items must point to at least itemCount entries and respect the invariant
  // documented by SESSION_HISTORY_VIEW_MAX_ITEMS.
  const SessionHistoryItemView *items = nullptr;
  int itemCount = 0;
  int selectedIndex = 0;
  bool detailOpen = false;
  SessionHistoryDetailView detail;
  bool deleteConfirmActive = false;
  bool deleteConfirmYesSelected = false;
};
