#include <ui/settings/wifi.h>

#include <Arduino.h>
#include <app/app_state.h>
#include <flow/wifi_flow.h>
#include <ui/confirm_overlay.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
unsigned long wifiLastDrawMs = 0;
constexpr unsigned long WIFI_DRAW_INTERVAL_MS = 250;

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

const char *staBadge(WifiStaUiState sta) {
  if (sta == WifiStaUiState::Connected)
    return "CONN";
  if (sta == WifiStaUiState::Connecting)
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

void drawWiFiScreen() {
  WifiFlowSnapshot wifi = wifiFlowSnapshot();
  bool hasSsid = wifi.staSsid && wifi.staSsid[0] != '\0';
  bool hasIp = wifi.staIp && wifi.staIp[0] != '\0';

  display.clearDisplay();
  drawHeader("Wi-Fi", wifi.setupMode ? setupBadge(wifi.provisionState)
                                     : staBadge(wifi.staState));
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, ui::layout::INFO_ROW1_Y);
  if (wifi.setupMode) {
    display.print("BLE: ");
    display.print(wifi.serviceName);
  } else {
    display.print("SSID: ");
    if (hasSsid) {
      display.print(wifi.staSsid);
    } else {
      display.print("-");
    }
  }

  display.setCursor(0, ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y);
  if (wifi.setupMode) {
    display.print("POP: ");
    display.print(wifi.pop);
  } else {
    display.print("IP: ");
    if (hasIp)
      display.print(wifi.staIp);
    else
      display.print("-");
  }

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 2);
  if (wifi.setupMode) {
    display.print("State: ");
    display.print(provisionStateText(wifi.provisionState));
  } else {
    display.print("State: ");
    if (wifi.connected) {
      display.print("CONNECTED");
    } else {
      display.print(staStatusText(wifi.staState));
    }
  }

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 3);
  if (wifi.setupMode) {
    if (wifi.provisionState == WifiProvisionUiState::Failed) {
      const char *reason = provisionFailReasonText(wifi.provisionFailReason);
      display.print("Fail: ");
      display.print(reason);
    } else if (wifi.provisionState == WifiProvisionUiState::NotSupported) {
      display.print("BLE not supported");
    } else {
      display.print("Use ESP BLE app");
    }
  } else {
    if (wifi.connected) {
      display.print("RSSI: ");
      if (wifi.rssi == 0) {
        display.print("-");
      } else {
        display.print(wifi.rssi);
        display.print(" dBm");
      }
    } else {
      display.print("Saved: yes");
    }
  }

  display.setCursor(0, 56);
  if (wifi.setupMode && wifi.provisionState == WifiProvisionUiState::Failed) {
    display.print("Hold: retry");
  } else if (wifi.setupMode) {
    display.print("Back: menu");
  } else {
    display.print("Hold: reset");
  }

  if (app.wifi.resetConfirm.active) {
    drawConfirmOverlay("Reset Wi-Fi?", app.wifi.resetConfirm);
  }

  display.display();
}
} // namespace

void updateWiFiScreen() {
  wifiFlowTick();

  unsigned long now = millis();
  if (now - wifiLastDrawMs >= WIFI_DRAW_INTERVAL_MS) {
    drawWiFiScreen();
    wifiLastDrawMs = now;
  }
}

void drawWiFi() {
  wifiFlowEnterScreen();
  wifiLastDrawMs = 0;
  drawWiFiScreen();
}

void stopWiFiProvisioning() { wifiFlowExitScreen(); }
