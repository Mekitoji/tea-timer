#include <controllers/wifi_controller.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/long_press.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <flow/wifi_flow.h>
#include <hw/pins.h>
#include <presentation/wifi_presenter.h>

namespace {
LongPressTracker wifiLongPress;
unsigned long wifiLastDrawMs = 0;
constexpr unsigned long WIFI_DRAW_INTERVAL_MS = 250;

void resetWiFiLongPressFlowState() {
  wifiLongPress.reset();
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
    wifiRender();
  }

  return true;
}

bool handleWiFiBackInput() {
  if (currentScreen != SCREEN_WIFI)
    return false;

  if (app.wifi.resetConfirm.active) {
    closeConfirm(app.wifi.resetConfirm);
    wifiRender();
    return true;
  }

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
    wifiRender();
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

  const WifiProvisionUiState provisionState = wifiProvisionState();
  const bool canRetryFailedProvisioning =
      provisionState == WifiProvisionUiState::Failed;
  const bool canResetSavedCredentials = wifiProvisionHasSavedCredentials();

  if (app.wifi.resetConfirm.active ||
      (!canRetryFailedProvisioning && !canResetSavedCredentials)) {
    resetWiFiLongPressFlowState();
    return;
  }

  const unsigned long now = millis();
  const bool down = (digitalRead(ENC_SW) == LOW);

  if (wifiLongPress.update(down, now, appcfg::WIFI_HOLD_MS) !=
      LongPressEvent::LongPressed)
    return;

  if (canRetryFailedProvisioning) {
    wifiRetryFailedProvisioning();
    wifiRender();
    return;
  }

  openConfirm(app.wifi.resetConfirm);
  wifiRender();
}

void updateWiFiScreen() {
  if (currentScreen != SCREEN_WIFI) {
    wifiLastDrawMs = 0;
    return;
  }

  wifiFlowTick();

  unsigned long now = millis();
  if (wifiLastDrawMs == 0) {
    wifiLastDrawMs = now;
    return;
  }
  if (now - wifiLastDrawMs < WIFI_DRAW_INTERVAL_MS)
    return;

  wifiRender();
  wifiLastDrawMs = now;
}
