#include <ui/info.h>

#include <Arduino.h>
#include <WiFi.h>
#include <app/app_state.h>
#include <flow/wifi_flow.h>
#include <ui/header.h>
#include <ui/layout.h>

namespace {
unsigned long wifiLastDrawMs = 0;
constexpr unsigned long WIFI_DRAW_INTERVAL_MS = 250;

const char *staStatusText(wl_status_t status) {
  switch (status) {
  case WL_CONNECTED:
    return "CONNECTED";
  case WL_CONNECT_FAILED:
    return "CONN_FAIL";
  case WL_CONNECTION_LOST:
    return "CONN_LOST";
  case WL_NO_SSID_AVAIL:
    return "NO_SSID";
  case WL_IDLE_STATUS:
    return "CONNECTING";
  case WL_DISCONNECTED:
    return "DISCONNECTED";
  default:
    return "UNKNOWN";
  }
}

const char *setupBadge() {
  switch (wifiProvisionState()) {
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

const char *staBadge(wl_status_t sta) {
  if (sta == WL_CONNECTED)
    return "CONN";
  if (sta == WL_IDLE_STATUS)
    return "CONN?";
  return "DISC";
}

void drawWiFiResetConfirmOverlay() {
  const int x = 8;
  const int y = 20;
  const int w = 112;
  const int h = 24;

  display.fillRect(x, y, w, h, SSD1306_BLACK);
  display.drawRect(x, y, w, h, SSD1306_WHITE);
  display.setCursor(x + 6, y + 4);
  display.print("Reset Wi-Fi?");
  display.setCursor(x + 6, y + 14);
  if (app.wifi.resetConfirm.yesSelected) {
    display.print("No [YES]");
  } else {
    display.print("[NO] Yes");
  }
}

void drawWiFiScreen() {
  bool hasSaved = wifiProvisionHasSavedCredentials();
  bool setupMode = !hasSaved;
  wl_status_t sta = WiFi.status();
  WifiProvisionUiState pstate = wifiProvisionState();
  IPAddress localIp = WiFi.localIP();
  bool connected =
      (sta == WL_CONNECTED) || (pstate == WifiProvisionUiState::Connected);

  String ssid = WiFi.SSID();
  const char *cachedSsid = wifiProvisionStaSsid();
  const char *cachedIp = wifiProvisionStaIp();
  bool hasCachedSsid = (cachedSsid && cachedSsid[0] != '\0');
  bool hasCachedIp = (cachedIp && cachedIp[0] != '\0');

  display.clearDisplay();
  drawHeader("Wi-Fi", setupMode ? setupBadge() : staBadge(connected ? WL_CONNECTED : sta));
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, ui::layout::INFO_ROW1_Y);
  if (setupMode) {
    display.print("BLE: ");
    display.print(wifiProvisionApSsid());
  } else {
    display.print("SSID: ");
    if (ssid.length() > 0) {
      display.print(ssid.c_str());
    } else if (hasCachedSsid) {
      display.print(cachedSsid);
    } else {
      display.print("-");
    }
  }

  display.setCursor(0, ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y);
  if (setupMode) {
    display.print("POP: ");
    display.print(wifiProvisionPop());
  } else {
    display.print("IP: ");
    if (localIp != IPAddress(static_cast<uint32_t>(0)))
      display.print(localIp);
    else if (hasCachedIp)
      display.print(cachedIp);
    else
      display.print("-");
  }

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 2);
  if (setupMode) {
    display.print("State: ");
    display.print(wifiProvisionStatusText());
  } else {
    display.print("State: ");
    if (connected) {
      display.print("CONNECTED");
    } else {
      display.print(staStatusText(sta));
    }
  }

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 3);
  if (setupMode) {
    if (pstate == WifiProvisionUiState::Failed) {
      const char *reason = wifiProvisionFailureReasonText();
      display.print("Fail: ");
      display.print((reason && reason[0]) ? reason : "RETRY");
    } else if (pstate == WifiProvisionUiState::NotSupported) {
      display.print("BLE not supported");
    } else {
      display.print("Use ESP BLE app");
    }
  } else {
    if (connected) {
      int rssi = WiFi.RSSI();
      display.print("RSSI: ");
      if (rssi == 0) {
        display.print("-");
      } else {
        display.print(rssi);
        display.print(" dBm");
      }
    } else {
      display.print("Saved: yes");
    }
  }

  display.setCursor(0, 56);
  if (setupMode) {
    display.print("Back: menu");
  } else {
    display.print("Hold: reset");
  }

  if (app.wifi.resetConfirm.active) {
    drawWiFiResetConfirmOverlay();
  }

  display.display();
}
} // namespace

void drawAbout() {
  display.clearDisplay();
  drawHeader("About Device");

  display.setCursor(0, ui::layout::INFO_ROW1_Y);
  display.print("Chip: ESP32-C3");

  display.setCursor(0, ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y);
  display.print("Flash: ");
  display.print(ESP.getFlashChipSize() / 1024 / 1024);
  display.print("MB");

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 2);
  display.print("Heap: ");
  display.print(ESP.getFreeHeap());

  display.setCursor(0,
                    ui::layout::INFO_ROW1_Y + ui::layout::INFO_ROW_STEP_Y * 3);
  display.print("FW: v1.0.0");

  display.display();
}

void updateWiFiScreen() {
  bool hasSaved = wifiProvisionHasSavedCredentials();
  if (!hasSaved && !wifiProvisionIsActive()) {
    wifiProvisionStart();
  }
  if (!hasSaved) {
    wifiProvisionUpdate();
  }

  unsigned long now = millis();
  if (now - wifiLastDrawMs >= WIFI_DRAW_INTERVAL_MS) {
    drawWiFiScreen();
    wifiLastDrawMs = now;
  }
}

void drawPowerSave(bool enabled) {
  display.clearDisplay();
  drawHeader("Power Save");

  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("Mode:");

  display.drawRect(44, 16, 36, 12, SSD1306_WHITE);
  display.setCursor(52, 18);
  display.print(enabled ? "ON" : "OFF");

  display.display();
}

void drawWiFi() {
  bool hasSaved = wifiProvisionHasSavedCredentials();
  if (!hasSaved && !wifiProvisionIsActive()) {
    wifiProvisionStart();
  }
  wifiLastDrawMs = 0;
  drawWiFiScreen();
}

void stopWiFiProvisioning() { wifiProvisionStop(); }
