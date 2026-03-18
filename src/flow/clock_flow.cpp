#include <flow/clock_flow.h>

#include <app/app_state.h>
#include <app/clock_time.h>
#include <flow/navigation_flow.h>
#include <flow/clock_runtime.h>
#include <ui.h>

namespace {
ClockRow nextRow(ClockRow row, bool plus) {
  if (plus) {
    if (row == ClockRow::Time)
      return ClockRow::Date;
    if (row == ClockRow::Date)
      return ClockRow::AutoSync;
    return ClockRow::Time;
  }

  if (row == ClockRow::AutoSync)
    return ClockRow::Date;
  if (row == ClockRow::Date)
    return ClockRow::Time;
  return ClockRow::AutoSync;
}
} // namespace

void clockRender() { drawClock(); }

void clockEnter() {
  clockRefreshStateFromSystemTime();
  app.clock.selectedRow = ClockRow::Time;
  app.clock.editMode = false;
  app.clock.editPart = 0;
}

void clockHandleEncoder(bool stepPlus, bool stepMinus) {
  if (!stepPlus && !stepMinus)
    return;

  const bool plus = stepPlus;

  if (!app.clock.editMode) {
    app.clock.selectedRow = nextRow(app.clock.selectedRow, plus);
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::Time) {
    if (app.clock.editPart == 0) {
      app.clock.draftHour += plus ? 1 : -1;
      if (app.clock.draftHour < 0)
        app.clock.draftHour = 23;
      if (app.clock.draftHour > 23)
        app.clock.draftHour = 0;
    } else {
      app.clock.draftMinute += plus ? 1 : -1;
      if (app.clock.draftMinute < 0)
        app.clock.draftMinute = 59;
      if (app.clock.draftMinute > 59)
        app.clock.draftMinute = 0;
    }
    app.clock.draftAutoSyncEnabled = false;
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::Date) {
    if (app.clock.editPart == 0) {
      app.clock.draftDay += plus ? 1 : -1;
    } else if (app.clock.editPart == 1) {
      app.clock.draftMonth += plus ? 1 : -1;
    } else {
      app.clock.draftYear += plus ? 1 : -1;
    }

    clockClampDraftDate(app.clock);
    app.clock.draftAutoSyncEnabled = false;
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::AutoSync) {
    app.clock.draftAutoSyncEnabled = !app.clock.draftAutoSyncEnabled;
    clockRender();
    return;
  }
}

void clockHandleSelect() {
  if (!app.clock.editMode) {
    app.clock.editMode = true;
    app.clock.editPart = 0;
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::Time) {
    if (app.clock.editPart == 0) {
      app.clock.editPart = 1;
    } else {
      app.clock.editMode = false;
      app.clock.editPart = 0;
    }
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::Date) {
    if (app.clock.editPart < 2) {
      app.clock.editPart++;
    } else {
      app.clock.editMode = false;
      app.clock.editPart = 0;
    }
    clockRender();
    return;
  }

  app.clock.editMode = false;
  app.clock.editPart = 0;
  clockRender();
}

void clockHandleBack() {
  bool previousAutoSyncEnabled = app.clock.autoSyncEnabled;
  bool manualTimeChanged = app.clock.year != app.clock.draftYear ||
                           app.clock.month != app.clock.draftMonth ||
                           app.clock.day != app.clock.draftDay ||
                           app.clock.hour != app.clock.draftHour ||
                           app.clock.minute != app.clock.draftMinute;

  clockApplyDraftToState(app.clock);

  if (manualTimeChanged) {
    app.clock.source = ClockSource::Manual;
    app.clock.autoSyncEnabled = false;
    app.clock.draftAutoSyncEnabled = false;
    clockCancelNtpSync();
  }

  time_t epoch = 0;
  if (clockBuildEpochFromDraft(app.clock, epoch)) {
    if (manualTimeChanged) {
      app.clock.timeValid = clockSetSystemTimeFromEpoch(epoch);
      app.clock.timeFreshThisBoot = app.clock.timeValid;
    }
    clockPersistState(epoch);
  } else if (manualTimeChanged) {
    app.clock.timeValid = false;
    app.clock.timeFreshThisBoot = false;
    clockPersistState(0);
  }

  if (!manualTimeChanged && !previousAutoSyncEnabled && app.clock.autoSyncEnabled) {
    clockRequestNtpSync();
  } else if (!app.clock.autoSyncEnabled) {
    clockCancelNtpSync();
  }

  app.clock.editMode = false;
  app.clock.editPart = 0;
  showSettingsScreen();
}
