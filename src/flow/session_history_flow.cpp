#include <flow/session_history_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/clock_time.h>
#include <app/long_press.h>
#include <flow/navigation_flow.h>
#include <presentation/session_history_presenter.h>
#include <storage/session_journal_store.h>

#include <cstring>

namespace {
LongPressTracker historyLongPress;

SessionJournal &journal() { return sessionJournalStoreScratch(); }

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

void renderHistory() {
  clampSelectedIndex();
  sessionHistoryRender(sessionHistorySnapshot());
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

SessionHistorySnapshot sessionHistorySnapshot() {
  SessionHistorySnapshot snapshot;
  snapshot.records = journal().records;
  snapshot.recordCount = journal().recordCount;
  snapshot.selectedIndex = app.history.selectedIndex;
  snapshot.detailOpen = app.history.detailOpen;
  snapshot.deleteConfirmActive = app.history.deleteConfirm.active;
  snapshot.deleteConfirmYesSelected = app.history.deleteConfirm.yesSelected;

  return snapshot;
}

void resetSessionHistoryLongPressState() {
  historyLongPress.reset();
}

void sessionHistoryEnter() {
  loadHistoryJournal();
  closeConfirm(app.history.deleteConfirm);
  app.history.detailOpen = false;
  app.history.selectedIndex = journal().recordCount - 1;
  navigateTo(SCREEN_SESSION_HISTORY);
  renderHistory();
}

void sessionHistoryHandleEncoder(bool stepPlus, bool stepMinus) {
  if (app.history.deleteConfirm.active) {
    if (stepPlus)
      setConfirmChoice(app.history.deleteConfirm, true);
    if (stepMinus)
      setConfirmChoice(app.history.deleteConfirm, false);
    renderHistory();
    return;
  }

  if (journal().recordCount <= 0)
    return;

  app.history.selectedIndex += stepPlus ? 1 : -1;
  renderHistory();
}

void sessionHistoryHandleSelect() {
  if (app.history.deleteConfirm.active) {
    if (app.history.deleteConfirm.yesSelected)
      deleteSelectedRecord();
    closeConfirm(app.history.deleteConfirm);
    renderHistory();
    return;
  }

  if (journal().recordCount <= 0)
    return;

  app.history.detailOpen = !app.history.detailOpen;
  renderHistory();
}

void sessionHistoryHandleBack() {
  if (app.history.deleteConfirm.active) {
    closeConfirm(app.history.deleteConfirm);
    renderHistory();
    return;
  }

  if (app.history.detailOpen) {
    app.history.detailOpen = false;
    renderHistory();
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
    renderHistory();
  }
}
