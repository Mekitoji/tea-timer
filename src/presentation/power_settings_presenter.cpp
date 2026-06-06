#include <presentation/power_settings_presenter.h>

#include <app/app_state.h>
#include <cstdio>
#include <ui/settings/power_save.h>

namespace {
void formatTimeoutLabel(unsigned long timeoutMs, char *buf, size_t bufSize) {
  unsigned long sec = timeoutMs / 1000UL;
  if (sec % 60UL == 0) {
    std::snprintf(buf, bufSize, "%lum", sec / 60UL);
  } else {
    std::snprintf(buf, bufSize, "%lus", sec);
  }
}

PowerSettingsView buildPowerSettingsView() {
  static char timeout[12];
  formatTimeoutLabel(app.power.draftDisplayOffTimeoutMs, timeout,
                     sizeof(timeout));

  PowerSettingsView view;
  view.enabled = app.power.draftEnabled;
  view.timeout = timeout;
  view.enabledSelected = app.power.selectedRow == PowerRow::Enabled;
  view.timeoutSelected = app.power.selectedRow == PowerRow::Timeout;
  view.editMode = app.power.editMode;
  return view;
}
} // namespace

void powerSettingsRender() { drawPowerSave(buildPowerSettingsView()); }
