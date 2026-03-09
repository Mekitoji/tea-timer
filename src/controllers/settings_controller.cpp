#include <controllers/settings_controller.h>

#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <storage/settings_store.h>
#include <ui.h>

namespace {
void handleSettingsSelect() {
  if (app.ui.settingsSelected == SETTINGS_WIFI) {
    showWiFiScreen();
  } else if (app.ui.settingsSelected == SETTINGS_ABOUT) {
    showAboutScreen();
  } else if (app.ui.settingsSelected == SETTINGS_POWER_SAVE) {
    showPowerSaveScreen();
    return;
  }
}
} // namespace

bool handleSettingsEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen == SCREEN_SETTINGS) {
    app.ui.settingsSelected += stepPlus ? 1 : -1;
    if (app.ui.settingsSelected < 0)
      app.ui.settingsSelected = settingsMenuCount - 1;
    if (app.ui.settingsSelected >= settingsMenuCount)
      app.ui.settingsSelected = 0;
    drawSettingsMenu();
    return true;
  }

  if (currentScreen == SCREEN_POWER_SAVE) {
    app.power.editEnabled = !app.power.editEnabled;
    drawPowerSave(app.power.editEnabled);
    return true;
  }

  return false;
}

bool handleSettingsSelectInput() {
  if (currentScreen == SCREEN_SETTINGS) {
    handleSettingsSelect();
    return true;
  }

  if (currentScreen == SCREEN_ABOUT) {
    showSettingsScreen();
    return true;
  }

  if (currentScreen == SCREEN_POWER_SAVE) {
    app.power.enabled = app.power.editEnabled;
    setPowerSavingEnabled(app.power.enabled);
    settingsStoreSavePowerSaveEnabled(app.power.enabled);
    showSettingsScreen();
    return true;
  }

  return false;
}

bool handleSettingsBackInput() {
  if (currentScreen == SCREEN_ABOUT || currentScreen == SCREEN_POWER_SAVE) {
    showSettingsScreen();
    return true;
  }

  if (currentScreen == SCREEN_SETTINGS) {
    showMenuScreen();
    return true;
  }

  return false;
}
