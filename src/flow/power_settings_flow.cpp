#include <flow/power_settings_flow.h>

#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <storage/settings_store.h>
#include <ui.h>

namespace {
constexpr unsigned long TIMEOUT_OPTIONS_MS[] = {15000UL, 30000UL, 60000UL,
                                                120000UL, 300000UL};
constexpr int TIMEOUT_OPTIONS_COUNT =
    sizeof(TIMEOUT_OPTIONS_MS) / sizeof(TIMEOUT_OPTIONS_MS[0]);

PowerRow nextRow(PowerRow row, bool plus) {
  if (plus) {
    return (row == PowerRow::Enabled) ? PowerRow::Timeout : PowerRow::Enabled;
  }
  return (row == PowerRow::Timeout) ? PowerRow::Enabled : PowerRow::Timeout;
}

int timeoutIndexForMs(unsigned long timeoutMs) {
  int bestIndex = 0;
  unsigned long bestDiff = (TIMEOUT_OPTIONS_MS[0] > timeoutMs)
                               ? (TIMEOUT_OPTIONS_MS[0] - timeoutMs)
                               : (timeoutMs - TIMEOUT_OPTIONS_MS[0]);

  for (int i = 1; i < TIMEOUT_OPTIONS_COUNT; i++) {
    unsigned long opt = TIMEOUT_OPTIONS_MS[i];
    unsigned long diff =
        (opt > timeoutMs) ? (opt - timeoutMs) : (timeoutMs - opt);
    if (diff < bestDiff) {
      bestDiff = diff;
      bestIndex = i;
    }
  }

  return bestIndex;
}

unsigned long nextTimeoutMs(unsigned long currentMs, bool plus) {
  int index = timeoutIndexForMs(currentMs);
  index += plus ? 1 : -1;
  if (index < 0)
    index = TIMEOUT_OPTIONS_COUNT - 1;
  if (index >= TIMEOUT_OPTIONS_COUNT)
    index = 0;
  return TIMEOUT_OPTIONS_MS[index];
}
} // namespace

void powerSettingsRender() { drawPowerSave(app.power); }

void powerSettingsEnter() {
  app.power.selectedRow = PowerRow::Enabled;
  app.power.editMode = false;
  app.power.draftEnabled = app.power.enabled;
  app.power.draftDisplayOffTimeoutMs = app.power.displayOffTimeoutMs;
}

void powerSettingsHandleEncoder(bool stepPlus, bool stepMinus) {
  if (!stepPlus && !stepMinus)
    return;

  const bool plus = stepPlus;

  if (!app.power.editMode) {
    app.power.selectedRow = nextRow(app.power.selectedRow, plus);
    powerSettingsRender();
    return;
  }

  if (app.power.selectedRow == PowerRow::Enabled) {
    app.power.draftEnabled = !app.power.draftEnabled;
  } else {
    app.power.draftDisplayOffTimeoutMs =
        nextTimeoutMs(app.power.draftDisplayOffTimeoutMs, plus);
  }

  powerSettingsRender();
}

void powerSettingsHandleSelect() {
  app.power.editMode = !app.power.editMode;
  powerSettingsRender();
}

void powerSettingsHandleBack() {
  app.power.enabled = app.power.draftEnabled;
  app.power.displayOffTimeoutMs = app.power.draftDisplayOffTimeoutMs;

  setPowerSavingEnabled(app.power.enabled);
  setDisplayIdleOffTimeoutMs(app.power.displayOffTimeoutMs);

  settingsStoreSavePowerSaveEnabled(app.power.enabled);
  settingsStoreSaveDisplayIdleOffMs(app.power.displayOffTimeoutMs);

  showSettingsScreen();
}
