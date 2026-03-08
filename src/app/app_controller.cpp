#include <app/app_controller.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <controllers/menu_controller.h>
#include <controllers/session_controller.h>
#include <controllers/settings_controller.h>
#include <controllers/timer_controller.h>
#include <controllers/wifi_controller.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <hw/input.h>
#include <ui/menu.h>

namespace {
bool shouldIgnoreInputByPowerGuard() {
  if (isWakeInputGuardActive())
    return true;
  if (markUserActivityAndConsumeIfWoke())
    return true;
  return false;
}
} // namespace

void handleEncoderByScreen(bool stepPlus, bool stepMinus) {
  if (!stepPlus && !stepMinus) {
    return;
  }
  if (shouldIgnoreInputByPowerGuard())
    return;

  if (handleMenuEncoderInput(stepPlus, stepMinus)) {
    return;
  }
  if (handleTimerEncoderInput(stepPlus, stepMinus)) {
    return;
  }
  if (handleSessionEncoderInput(stepPlus, stepMinus)) {
    return;
  }
  if (handleSettingsEncoderInput(stepPlus, stepMinus)) {
    return;
  }
  if (handleWiFiEncoderInput(stepPlus, stepMinus)) {
    return;
  }
}

void handleBackButton() {
  if (!backButtonPressedEvent())
    return;

  if (shouldIgnoreInputByPowerGuard())
    return;

  if (handleWiFiBackInput()) {
    return;
  }

  if (handleSessionBackInput()) {
    return;
  }

  if (handleTimerBackInput()) {
    return;
  }

  if (handleSettingsBackInput()) {
    return;
  }

  if (goBack())
    return;
}

void handleSelectButton() {
  if (buttonPressedEvent()) {
    if (shouldIgnoreInputByPowerGuard())
      return;

    if (handleMenuSelectInput())
      return;
    if (handleTimerSelectInput())
      return;
    if (handleSessionSelectInput())
      return;
    if (handleSettingsSelectInput())
      return;
    if (handleWiFiSelectInput())
      return;

    navigateTo(SCREEN_MENU);
    drawMenu();
  }
}

void handleLongPress() {
  handleSessionLongPressInput();
  handleWiFiLongPressInput();
  handleTimerLongPressInput();
}
