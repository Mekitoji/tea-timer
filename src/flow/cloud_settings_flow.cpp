#include <flow/cloud_settings_flow.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/long_press.h>
#include <flow/device_pairing_flow.h>
#include <flow/device_sync_flow.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <hw/pins.h>
#include <ui.h>

namespace {
LongPressTracker cloudLongPress;

bool isPairingActive(DevicePairingFlowState state) {
  return state == DevicePairingFlowState::Creating ||
         state == DevicePairingFlowState::Pending;
}

void resetCloudLongPressState() { cloudLongPress.reset(); }
} // namespace

void cloudSettingsEnter() {
  closeConfirm(app.cloud.unpairConfirm);
  resetCloudLongPressState();
}

void cloudSettingsRender() { drawCloud(); }

void cloudSettingsHandleEncoder(bool stepPlus, bool stepMinus) {
  if (!app.cloud.unpairConfirm.active)
    return;

  if (stepPlus)
    setConfirmChoice(app.cloud.unpairConfirm, true);
  if (stepMinus)
    setConfirmChoice(app.cloud.unpairConfirm, false);
  drawCloud();
}

void cloudSettingsHandleSelect() {
  if (app.cloud.unpairConfirm.active) {
    bool doUnpair = app.cloud.unpairConfirm.yesSelected;
    closeConfirm(app.cloud.unpairConfirm);
    if (doUnpair) {
      unpairDevice();
    }
    drawCloud();
    return;
  }

  DevicePairingSnapshot pairing;
  devicePairingSnapshot(pairing);

  if (isPairingActive(pairing.state)) {
    cancelDevicePairing();
    drawCloud();
    return;
  }

  if (!pairing.paired) {
    startDevicePairing();
    drawCloud();
    return;
  }

  requestDeviceSyncNow();
  updateDeviceSyncFlow();
  drawCloud();
}

void cloudSettingsHandleBack() {
  if (app.cloud.unpairConfirm.active) {
    closeConfirm(app.cloud.unpairConfirm);
    drawCloud();
    return;
  }

  showSettingsScreen();
}

void cloudSettingsHandleLongPressInput() {
  if (isWakeInputGuardActive())
    return;

  if (currentScreen != SCREEN_CLOUD) {
    resetCloudLongPressState();
    return;
  }

  DevicePairingSnapshot pairing;
  devicePairingSnapshot(pairing);
  if (!pairing.paired || app.cloud.unpairConfirm.active) {
    resetCloudLongPressState();
    return;
  }

  const bool down = digitalRead(ENC_SW) == LOW;
  if (cloudLongPress.update(down, millis(), appcfg::CLOUD_HOLD_MS) !=
      LongPressEvent::LongPressed)
    return;

  openConfirm(app.cloud.unpairConfirm);
  drawCloud();
}
