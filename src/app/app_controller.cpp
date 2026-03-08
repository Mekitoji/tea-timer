#include "app/app_controller.h"

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/tea_config.h>
#include <controllers/session_controller.h>
#include <controllers/wifi_controller.h>
#include <flow/menu_flow.h>
#include <flow/power_flow.h>
#include <flow/timer_flow.h>
#include <hw/input.h>
#include <hw/pins.h>
#include <ui.h>

void handleEncoderByScreen(bool stepPlus, bool stepMinus) {
  if (stepPlus || stepMinus) {
    if (isWakeInputGuardActive())
      return;
    if (markUserActivityAndConsumeIfWoke())
      return;
    if (currentScreen == SCREEN_MENU) {
      selected += stepPlus ? 1 : -1;
      if (selected < 0)
        selected = menuCount - 1;
      if (selected >= menuCount)
        selected = 0;
      drawMenu();
    } else if (currentScreen == SCREEN_TIMER) {
      int step = encoderAccelStepForTimestamp(millis());
      int delta = stepPlus ? step : -step;
      timerAdjustByEncoderDelta(delta);
    } else if (handleSessionEncoderInput(stepPlus, stepMinus)) {
      return;
    } else if (currentScreen == SCREEN_SETTINGS) {
      settingsSelected += stepPlus ? 1 : -1;
      if (settingsSelected < 0)
        settingsSelected = settingsMenuCount - 1;
      if (settingsSelected >= settingsMenuCount)
        settingsSelected = 0;
      drawSettingsMenu();
    } else if (handleWiFiEncoderInput(stepPlus, stepMinus)) {
      return;
    } else if (currentScreen == SCREEN_POWER_SAVE) {
      powerSaveEditEnabled = !powerSaveEditEnabled;
      drawPowerSave(powerSaveEditEnabled);
    }
  }
}

void handleBackButton() {
  if (!backButtonPressedEvent())
    return;

  if (isWakeInputGuardActive())
    return;
  if (markUserActivityAndConsumeIfWoke())
    return;

  if (handleWiFiBackInput()) {
    return;
  }

  if (handleSessionBackInput()) {
    return;
  }

  if (currentScreen == SCREEN_ABOUT || currentScreen == SCREEN_POWER_SAVE) {
    currentScreen = SCREEN_SETTINGS;
    drawSettingsMenu();
    return;
  }

  if (currentScreen == SCREEN_TIMER) {
    applyTimerPresetSec(timerTotalSec);
    prefs.putInt(appcfg::PREFS_DURATION_KEY, timerDuration);
    resetSingleTimerRuntimeState();
  }

  if (currentScreen != SCREEN_MENU) {
    goToMenu();
  }
}

void handleSelectButton() {
  if (buttonPressedEvent()) {
    if (isWakeInputGuardActive())
      return;
    if (markUserActivityAndConsumeIfWoke())
      return;

    if (currentScreen == SCREEN_MENU) {
      handleMenuSelect();
    } else if (currentScreen == SCREEN_TIMER) {
      // handled by handleTimerButton
    } else if (handleSessionSelectInput()) {
      return;
    } else if (currentScreen == SCREEN_SETTINGS) {
      handleSettingsSelect();
    } else if (handleWiFiSelectInput()) {
      return;
    } else if (currentScreen == SCREEN_ABOUT) {
      currentScreen = SCREEN_SETTINGS;
      drawSettingsMenu();
    } else if (currentScreen == SCREEN_POWER_SAVE) {
      powerSaveEnabled = powerSaveEditEnabled;
      setPowerSavingEnabled(powerSaveEnabled);
      prefs.putBool(appcfg::PREFS_POWER_SAVE_KEY, powerSaveEnabled);
      currentScreen = SCREEN_SETTINGS;
      drawSettingsMenu();
    } else {
      goToMenu();
    }
  }
}

void handleSessionLongPress() { handleSessionLongPressInput(); }

void handleWiFiLongPress() { handleWiFiLongPressInput(); }

void handleTimerButton() {
  if (currentScreen != SCREEN_TIMER) {
    timerIgnoreReleaseAfterEnter = false;
    resetTimerButtonFlowState();
    return;
  }

  if (isWakeInputGuardActive())
    return;

  const unsigned long now = millis();
  const bool down = (digitalRead(ENC_SW) == LOW);
  processTimerButtonInput(down, now);
}
