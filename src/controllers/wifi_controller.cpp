#include <controllers/wifi_controller.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/long_press.h>
#include <flow/navigation_flow.h>
#include <flow/power_flow.h>
#include <flow/wifi_flow.h>
#include <hw/pins.h>
#include <ui/settings/wifi.h>

#include <cstdio>

namespace {
LongPressTracker wifiLongPress;
unsigned long wifiLastDrawMs = 0;
constexpr unsigned long WIFI_DRAW_INTERVAL_MS = 250;
char wifiRow1[48];
char wifiRow2[48];
char wifiRow3[48];
char wifiRow4[48];

void resetWiFiLongPressFlowState() {
  wifiLongPress.reset();
}

const char *staStatusText(WifiStaUiState status) {
  switch (status) {
  case WifiStaUiState::Connected:
    return "CONNECTED";
  case WifiStaUiState::ConnectFailed:
    return "CONN_FAIL";
  case WifiStaUiState::ConnectionLost:
    return "CONN_LOST";
  case WifiStaUiState::NoSsid:
    return "NO_SSID";
  case WifiStaUiState::Connecting:
    return "CONNECTING";
  case WifiStaUiState::Disconnected:
    return "DISCONNECTED";
  case WifiStaUiState::Unknown:
  default:
    return "UNKNOWN";
  }
}

const char *setupBadge(WifiProvisionUiState state) {
  switch (state) {
  case WifiProvisionUiState::Connected:
    return "CONN";
  case WifiProvisionUiState::Failed:
    return "FAIL";
  case WifiProvisionUiState::NotSupported:
    return "NOBLE";
  default:
    return "SETUP";
  }
}

const char *staBadge(WifiStaUiState state) {
  if (state == WifiStaUiState::Connected)
    return "CONN";
  if (state == WifiStaUiState::Connecting)
    return "CONN?";
  return "DISC";
}

const char *provisionStateText(WifiProvisionUiState state) {
  switch (state) {
  case WifiProvisionUiState::Idle:
    return "IDLE";
  case WifiProvisionUiState::WaitingCredentials:
    return "BLE_WAIT";
  case WifiProvisionUiState::Connecting:
    return "CONNECTING";
  case WifiProvisionUiState::Connected:
    return "CONNECTED";
  case WifiProvisionUiState::Failed:
    return "FAILED";
  case WifiProvisionUiState::NotSupported:
    return "NO_BLE";
  }
  return "IDLE";
}

const char *provisionFailReasonText(WifiProvisionFailReason reason) {
  switch (reason) {
  case WifiProvisionFailReason::AuthError:
    return "AUTH_ERR";
  case WifiProvisionFailReason::ApNotFound:
    return "AP_NOT_FOUND";
  case WifiProvisionFailReason::Unknown:
    return "UNKNOWN";
  case WifiProvisionFailReason::None:
  default:
    return "RETRY";
  }
}

WifiSettingsView buildWifiSettingsView() {
  WifiFlowSnapshot wifi = wifiFlowSnapshot();
  const char *ssid = wifi.staSsid && wifi.staSsid[0] ? wifi.staSsid : "-";
  const char *ip = wifi.staIp && wifi.staIp[0] ? wifi.staIp : "-";

  WifiSettingsView view;
  view.badge =
      wifi.setupMode ? setupBadge(wifi.provisionState) : staBadge(wifi.staState);
  view.resetConfirmActive = app.wifi.resetConfirm.active;
  view.resetConfirmYesSelected = app.wifi.resetConfirm.yesSelected;

  if (wifi.setupMode) {
    std::snprintf(wifiRow1, sizeof(wifiRow1), "BLE: %s", wifi.serviceName);
    std::snprintf(wifiRow2, sizeof(wifiRow2), "POP: %s", wifi.pop);
    std::snprintf(wifiRow3, sizeof(wifiRow3), "State: %s",
                  provisionStateText(wifi.provisionState));
    if (wifi.provisionState == WifiProvisionUiState::Failed) {
      std::snprintf(wifiRow4, sizeof(wifiRow4), "Fail: %s",
                    provisionFailReasonText(wifi.provisionFailReason));
      view.footer = "Hold: retry";
    } else if (wifi.provisionState == WifiProvisionUiState::NotSupported) {
      std::snprintf(wifiRow4, sizeof(wifiRow4), "BLE not supported");
      view.footer = "Back: menu";
    } else {
      std::snprintf(wifiRow4, sizeof(wifiRow4), "Use ESP BLE app");
      view.footer = "Back: menu";
    }
  } else {
    std::snprintf(wifiRow1, sizeof(wifiRow1), "SSID: %s", ssid);
    std::snprintf(wifiRow2, sizeof(wifiRow2), "IP: %s", ip);
    std::snprintf(wifiRow3, sizeof(wifiRow3), "State: %s",
                  wifi.connected ? "CONNECTED" : staStatusText(wifi.staState));
    if (!wifi.connected) {
      std::snprintf(wifiRow4, sizeof(wifiRow4), "Saved: yes");
    } else if (wifi.rssi == 0) {
      std::snprintf(wifiRow4, sizeof(wifiRow4), "RSSI: -");
    } else {
      std::snprintf(wifiRow4, sizeof(wifiRow4), "RSSI: %d dBm", wifi.rssi);
    }
    view.footer = "Hold: reset";
  }

  view.row1 = wifiRow1;
  view.row2 = wifiRow2;
  view.row3 = wifiRow3;
  view.row4 = wifiRow4;
  return view;
}
} // namespace

void wifiRender() { drawWiFi(buildWifiSettingsView()); }

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
