#include <controllers/wifi_controller.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <flow/wifi_flow.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
unsigned long wifiHoldStartMs = 0;
bool wifiWasDown = false;
bool wifiLongPressFired = false;

void resetWiFiLongPressFlowState() {
  wifiWasDown = false;
  wifiLongPressFired = false;
}
} // namespace

bool handleWiFiEncoderInput(bool stepPlus, bool stepMinus) {
  if (currentScreen != SCREEN_WIFI)
    return false;

  if (app.wifi.resetConfirm.active) {
    if (stepPlus)
      setConfirmChoice(app.wifi.resetConfirm, true);
    if (stepMinus)
      setConfirmChoice(app.wifi.resetConfirm, false);
    drawWiFi();
  }

  return true;
}

bool handleWiFiBackInput() {
  if (currentScreen != SCREEN_WIFI)
    return false;

  if (app.wifi.resetConfirm.active) {
    closeConfirm(app.wifi.resetConfirm);
    drawWiFi();
    return true;
  }

  stopWiFiProvisioning();
  showSettingsScreen();
  return true;
}

bool handleWiFiSelectInput() {
  if (currentScreen != SCREEN_WIFI)
    return false;

  if (app.wifi.resetConfirm.active) {
    bool doReset = app.wifi.resetConfirm.yesSelected;
    closeConfirm(app.wifi.resetConfirm);
    if (doReset) {
      wifiResetCredentialsAndStartProvisioning();
    }
    drawWiFi();
  }

  return true;
}

void handleWiFiLongPressInput() {
  if (isWakeInputGuardActive())
    return;

  if (currentScreen != SCREEN_WIFI) {
    resetWiFiLongPressFlowState();
    return;
  }

  if (app.wifi.resetConfirm.active || !wifiProvisionHasSavedCredentials()) {
    resetWiFiLongPressFlowState();
    return;
  }

  const unsigned long now = millis();
  const bool down = (digitalRead(ENC_SW) == LOW);

  if (down && !wifiWasDown) {
    wifiWasDown = true;
    wifiHoldStartMs = now;
    wifiLongPressFired = false;
  }

  if (!down && wifiWasDown) {
    wifiWasDown = false;
    wifiLongPressFired = false;
    return;
  }

  if (!down || !wifiWasDown || wifiLongPressFired)
    return;

  if (now - wifiHoldStartMs < appcfg::WIFI_HOLD_MS)
    return;

  wifiLongPressFired = true;
  openConfirm(app.wifi.resetConfirm);
  drawWiFi();
}
