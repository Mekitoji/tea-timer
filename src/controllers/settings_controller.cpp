#include <controllers/settings_controller.h>

#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <flow/power_settings_flow.h>
#include <flow/sound_settings_flow.h>
#include <ui.h>

namespace {
void handleSettingsSelect() {
  if (app.ui.settingsSelected == SETTINGS_WIFI) {
    showWiFiScreen();
  } else if (app.ui.settingsSelected == SETTINGS_ABOUT) {
    showAboutScreen();
  } else if (app.ui.settingsSelected == SETTINGS_SOUND) {
    soundSettingsEnter();
    showSoundScreen();
    return;
  } else if (app.ui.settingsSelected == SETTINGS_POWER_SAVE) {
    powerSettingsEnter();
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

  if (currentScreen == SCREEN_SOUND) {
    soundSettingsHandleEncoder(stepPlus, stepMinus);
    return true;
  }

  if (currentScreen == SCREEN_POWER_SAVE) {
    powerSettingsHandleEncoder(stepPlus, stepMinus);
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

  if (currentScreen == SCREEN_SOUND) {
    soundSettingsHandleSelect();
    return true;
  }

  if (currentScreen == SCREEN_POWER_SAVE) {
    powerSettingsHandleSelect();
    return true;
  }

  return false;
}

bool handleSettingsBackInput() {
  if (currentScreen == SCREEN_ABOUT) {
    showSettingsScreen();
    return true;
  }

  if (currentScreen == SCREEN_POWER_SAVE) {
    powerSettingsHandleBack();
    return true;
  }

  if (currentScreen == SCREEN_SETTINGS) {
    showMenuScreen();
    return true;
  }

  if (currentScreen == SCREEN_SOUND) {
    soundSettingsHandleBack();
    return true;
  }

  return false;
}
