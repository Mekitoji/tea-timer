#include <controllers/settings_controller.h>

#include <app/app_config.h>
#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <ui.h>

namespace {
void handleSettingsSelect() {
  if (settingsSelected == SETTINGS_WIFI) {
    navigateTo(SCREEN_WIFI);
    drawWiFi();
  } else if (settingsSelected == SETTINGS_ABOUT) {
    navigateTo(SCREEN_ABOUT);
    drawAbout();
  } else if (settingsSelected == SETTINGS_POWER_SAVE) {
    navigateTo(SCREEN_POWER_SAVE);
    powerSaveEditEnabled = powerSaveEnabled;
    drawPowerSave(powerSaveEditEnabled);
    return;
  }
}
} // namespace

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
    navigateTo(SCREEN_SETTINGS);
    drawSettingsMenu();
    return true;
  }

  if (currentScreen == SCREEN_POWER_SAVE) {
    powerSaveEnabled = powerSaveEditEnabled;
    setPowerSavingEnabled(powerSaveEnabled);
    prefs.putBool(appcfg::PREFS_POWER_SAVE_KEY, powerSaveEnabled);
    navigateTo(SCREEN_SETTINGS);
    drawSettingsMenu();
    return true;
  }

  return false;
}

bool handleSettingsBackInput() {
  if (currentScreen == SCREEN_ABOUT || currentScreen == SCREEN_POWER_SAVE) {
    navigateTo(SCREEN_SETTINGS);
    drawSettingsMenu();
    return true;
  }

  if (currentScreen == SCREEN_SETTINGS) {
    navigateTo(SCREEN_MENU);
    drawMenu();
    return true;
  }

  return false;
}
