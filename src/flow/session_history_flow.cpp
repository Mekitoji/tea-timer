#include <flow/session_history_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/clock_time.h>
#include <app/long_press.h>
#include <flow/navigation_flow.h>
#include <storage/session_journal_store.h>
#include <ui.h>

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
  drawSessionHistory(journal(), app.history);
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
