#include <controllers/settings_controller.h>

#include <app/app_config.h>
#include <app/app_state.h>
#include <flow/menu_flow.h>
#include <flow/power_flow.h>
#include <ui.h>

bool handleSettingsEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen == SCREEN_SETTINGS) {
    settingsSelected += stepPlus ? 1 : -1;
    if (settingsSelected < 0)
      settingsSelected = settingsMenuCount - 1;
    if (settingsSelected >= settingsMenuCount)
      settingsSelected = 0;
    drawSettingsMenu();
    return true;
  }

  if (currentScreen == SCREEN_POWER_SAVE) {
    powerSaveEditEnabled = !powerSaveEditEnabled;
    drawPowerSave(powerSaveEditEnabled);
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
    currentScreen = SCREEN_SETTINGS;
    drawSettingsMenu();
    return true;
  }

  if (currentScreen == SCREEN_POWER_SAVE) {
    powerSaveEnabled = powerSaveEditEnabled;
    setPowerSavingEnabled(powerSaveEnabled);
    prefs.putBool(appcfg::PREFS_POWER_SAVE_KEY, powerSaveEnabled);
    currentScreen = SCREEN_SETTINGS;
    drawSettingsMenu();
    return true;
  }

  return false;
}

bool handleSettingsBackInput() {
  if (currentScreen == SCREEN_ABOUT || currentScreen == SCREEN_POWER_SAVE) {
    currentScreen = SCREEN_SETTINGS;
    drawSettingsMenu();
    return true;
  }

  if (currentScreen == SCREEN_SETTINGS) {
    goToMenu();
    return true;
  }

  return false;
}
