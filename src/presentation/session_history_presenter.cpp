#include <presentation/session_history_presenter.h>

#include <app/session_log.h>
#include <cstdio>
#include <ctime>
#include <ui/session_history.h>

namespace {
SessionHistoryItemView historyItemViews[SESSION_HISTORY_VIEW_MAX_ITEMS];
char selectedStartedAt[14];
char selectedFinishedAt[14];

void formatEpoch(unsigned long epoch, char *out, size_t outSize) {
  if (!out || outSize == 0)
    return;

  if (epoch == 0) {
    std::snprintf(out, outSize, "--");
    return;
  }

  time_t value = static_cast<time_t>(epoch);
  std::tm tmValue = {};
  localtime_r(&value, &tmValue);
  std::snprintf(out, outSize, "%02d/%02d %02d:%02d", tmValue.tm_mday,
                tmValue.tm_mon + 1, tmValue.tm_hour, tmValue.tm_min);
}

SessionHistoryView buildHistoryView(const SessionHistorySnapshot &history) {
  SessionHistoryView view;
  view.items = historyItemViews;
  view.itemCount = history.recordCount;
  view.selectedIndex = history.selectedIndex;
  view.detailOpen = history.detailOpen;
  view.deleteConfirmActive = history.deleteConfirmActive;
  view.deleteConfirmYesSelected = history.deleteConfirmYesSelected;

  if (view.itemCount > SESSION_HISTORY_VIEW_MAX_ITEMS)
    view.itemCount = SESSION_HISTORY_VIEW_MAX_ITEMS;

  for (int i = 0; i < view.itemCount; i++) {
    const SessionLogRecord &record = history.records[i];
    historyItemViews[i].title =
        record.presetName[0] ? record.presetName : "Session";
    historyItemViews[i].completedInfusions = record.completedInfusionCount;
    historyItemViews[i].finishedEarly = record.finishedEarly;
  }

  if (view.selectedIndex < 0 || view.selectedIndex >= view.itemCount)
    return view;

  const SessionLogRecord &selected = history.records[view.selectedIndex];
  formatEpoch(selected.startedAt, selectedStartedAt, sizeof(selectedStartedAt));
  formatEpoch(selected.finishedAt, selectedFinishedAt,
              sizeof(selectedFinishedAt));

  view.detail.title =
      selected.presetName[0] ? selected.presetName : "Session";
  view.detail.startedAt = selectedStartedAt;
  view.detail.finishedAt = selectedFinishedAt;
  view.detail.completedInfusions = selected.completedInfusionCount;
  view.detail.rinseSec = selected.rinseSec;
  view.detail.finishedEarly = selected.finishedEarly;
  return view;
}
} // namespace

void sessionHistoryRender(const SessionHistorySnapshot &history) {
  drawSessionHistory(buildHistoryView(history));
}
