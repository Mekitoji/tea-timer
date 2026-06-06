#include <flow/session_history_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/clock_time.h>
#include <app/long_press.h>
#include <flow/navigation_flow.h>
#include <storage/session_journal_store.h>
#include <ui.h>

#include <cstdio>
#include <cstring>
#include <ctime>

namespace {
LongPressTracker historyLongPress;
SessionHistoryItemView historyItemViews[session_storage::MAX_RECORDS];
char selectedStartedAt[14];
char selectedFinishedAt[14];

SessionJournal &journal() { return sessionJournalStoreScratch(); }

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

SessionHistoryView buildHistoryView() {
  SessionHistoryView view;
  view.items = historyItemViews;
  view.itemCount = journal().recordCount;
  view.selectedIndex = app.history.selectedIndex;
  view.detailOpen = app.history.detailOpen;
  view.deleteConfirmActive = app.history.deleteConfirm.active;
  view.deleteConfirmYesSelected = app.history.deleteConfirm.yesSelected;

  for (int i = 0; i < journal().recordCount; i++) {
    const SessionLogRecord &record = journal().records[i];
    historyItemViews[i].title =
        record.presetName[0] ? record.presetName : "Session";
    historyItemViews[i].completedInfusions = record.completedInfusionCount;
    historyItemViews[i].finishedEarly = record.finishedEarly;
  }

  if (view.selectedIndex < 0 || view.selectedIndex >= view.itemCount)
    return view;

  const SessionLogRecord &selected = journal().records[view.selectedIndex];
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

void loadHistoryJournal() {
  if (!sessionJournalLoad(journal()))
    sessionJournalReset(journal());
}

void clampSelectedIndex() {
  if (journal().recordCount <= 0) {
    app.history.selectedIndex = 0;
    app.history.detailOpen = false;
    closeConfirm(app.history.deleteConfirm);
    return;
  }

  if (app.history.selectedIndex < 0)
    app.history.selectedIndex = journal().recordCount - 1;
  if (app.history.selectedIndex >= journal().recordCount)
    app.history.selectedIndex = 0;
}

void deleteSelectedRecord() {
  if (journal().recordCount <= 0)
    return;

  clampSelectedIndex();

  char id[sizeof(journal().records[0].id)] = {};
  std::strncpy(id, journal().records[app.history.selectedIndex].id,
               sizeof(id) - 1);

  if (sessionJournalDeleteRecordById(journal(), id,
                                     clockCurrentEpochOrZero())) {
    sessionJournalSave(journal());
    loadHistoryJournal();
    clampSelectedIndex();
  }
}
} // namespace

void resetSessionHistoryLongPressState() {
  historyLongPress.reset();
}

void sessionHistoryEnter() {
  loadHistoryJournal();
  closeConfirm(app.history.deleteConfirm);
  app.history.detailOpen = false;
  app.history.selectedIndex = journal().recordCount - 1;
  clampSelectedIndex();
  navigateTo(SCREEN_SESSION_HISTORY);
  sessionHistoryRender();
}

void sessionHistoryRender() {
  clampSelectedIndex();
  drawSessionHistory(buildHistoryView());
}

void sessionHistoryHandleEncoder(bool stepPlus, bool stepMinus) {
  if (app.history.deleteConfirm.active) {
    if (stepPlus)
      setConfirmChoice(app.history.deleteConfirm, true);
    if (stepMinus)
      setConfirmChoice(app.history.deleteConfirm, false);
    sessionHistoryRender();
    return;
  }

  if (journal().recordCount <= 0)
    return;

  app.history.selectedIndex += stepPlus ? 1 : -1;
  clampSelectedIndex();
  sessionHistoryRender();
}

void sessionHistoryHandleSelect() {
  if (app.history.deleteConfirm.active) {
    if (app.history.deleteConfirm.yesSelected)
      deleteSelectedRecord();
    closeConfirm(app.history.deleteConfirm);
    sessionHistoryRender();
    return;
  }

  if (journal().recordCount <= 0)
    return;

  app.history.detailOpen = !app.history.detailOpen;
  sessionHistoryRender();
}

void sessionHistoryHandleBack() {
  if (app.history.deleteConfirm.active) {
    closeConfirm(app.history.deleteConfirm);
    sessionHistoryRender();
    return;
  }

  if (app.history.detailOpen) {
    app.history.detailOpen = false;
    sessionHistoryRender();
    return;
  }

  showMenuScreen();
}

void sessionHistoryHandleLongPress(bool down, unsigned long nowMs) {
  if (currentScreen != SCREEN_SESSION_HISTORY || journal().recordCount <= 0) {
    resetSessionHistoryLongPressState();
    return;
  }

  if (app.history.deleteConfirm.active) {
    resetSessionHistoryLongPressState();
    return;
  }

  LongPressEvent event =
      historyLongPress.update(down, nowMs, appcfg::HISTORY_HOLD_MS);
  if (event == LongPressEvent::ShortReleased) {
    sessionHistoryHandleSelect();
  } else if (event == LongPressEvent::LongPressed) {
    openConfirm(app.history.deleteConfirm);
    sessionHistoryRender();
  }
}
