#include <presentation/clock_presenter.h>

#include <app/app_state.h>
#include <cstdio>
#include <flow/clock_runtime.h>
#include <ui/settings/clock.h>

namespace {
const char *syncStatusBadge(ClockSyncUiState state) {
  switch (state) {
  case ClockSyncUiState::Off:
    return "OFF";
  case ClockSyncUiState::Synced:
    return "OK";
  case ClockSyncUiState::Syncing:
    return "SYNC";
  case ClockSyncUiState::WaitingWifi:
    return "WIFI?";
  case ClockSyncUiState::Failed:
    return "FAIL";
  case ClockSyncUiState::WaitingRetry:
  case ClockSyncUiState::Waiting:
  default:
    return "WAIT";
  }
}

ClockSettingsView buildClockSettingsView() {
  static char time[8];
  static char date[16];
  std::snprintf(time, sizeof(time), "%02d:%02d", app.clock.draftHour,
                app.clock.draftMinute);
  std::snprintf(date, sizeof(date), "%02d-%02d-%04d", app.clock.draftDay,
                app.clock.draftMonth, app.clock.draftYear);

  ClockSettingsView view;
  view.badge = syncStatusBadge(clockSyncUiState());
  view.time = time;
  view.date = date;
  view.autoSync = app.clock.draftAutoSyncEnabled ? "ON" : "OFF";
  view.timeSelected = app.clock.selectedRow == ClockRow::Time;
  view.dateSelected = app.clock.selectedRow == ClockRow::Date;
  view.autoSyncSelected = app.clock.selectedRow == ClockRow::AutoSync;
  view.editMode = app.clock.editMode;
  return view;
}
} // namespace

void clockRender() { drawClock(buildClockSettingsView()); }
