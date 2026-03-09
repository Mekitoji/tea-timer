#include <flow/wifi_flow.h>

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiProv.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <cstdio>
#include <cstring>
#include <esp_wifi.h>
#include <wifi_provisioning/manager.h>

namespace {
#define WIFI_LOG(fmt, ...) Serial.printf("[wifi] " fmt "\n", ##__VA_ARGS__)

constexpr char PROV_POP[] = "teatimer";

WifiProvisionUiState provisionState = WifiProvisionUiState::Idle;
WifiProvisionFailReason provisionFailReason = WifiProvisionFailReason::None;

char serviceNameBuf[32] = "PROV_TEA";
char staSsidBuf[33] = "";
char staIpBuf[24] = "";

char pendingSsid[33] = "";

bool hasSavedCredentials = false;

bool eventHandlerRegistered = false;
bool provisioningSessionActive = false;
unsigned long lastReconnectMs = 0;

void copyToBuf(char *dst, size_t dstSize, const char *src) {
  if (!dst || dstSize == 0)
    return;
  if (!src) {
    dst[0] = '\0';
    return;
  }
  std::snprintf(dst, dstSize, "%s", src);
}

void setState(WifiProvisionUiState next) {
  provisionState = next;

  if (provisionState != WifiProvisionUiState::Failed) {
    provisionFailReason = WifiProvisionFailReason::None;
  }

  WIFI_LOG("state=%d", static_cast<int>(provisionState));
}

bool hasSystemStaCredentials() {
  wifi_config_t conf = {};

  if (esp_wifi_get_config(WIFI_IF_STA, &conf) != ESP_OK)
    return false;

  return conf.sta.ssid[0] != '\0';
}

void refreshSavedCredentialsFlag() {
  hasSavedCredentials = hasSystemStaCredentials();
}

void buildServiceName() {
  uint64_t chipId = ESP.getEfuseMac();
  uint8_t b3 = static_cast<uint8_t>((chipId >> 16) & 0xFF);
  uint8_t b4 = static_cast<uint8_t>((chipId >> 8) & 0xFF);
  uint8_t b5 = static_cast<uint8_t>(chipId & 0xFF);
  std::snprintf(serviceNameBuf, sizeof(serviceNameBuf), "PROV_%02X%02X%02X", b3,
                b4, b5);
}

void onProvisionEvent(arduino_event_t *event) {
  if (!event)
    return;

  switch (event->event_id) {
  case ARDUINO_EVENT_PROV_START:
    provisioningSessionActive = true;
    setState(WifiProvisionUiState::WaitingCredentials);
    WIFI_LOG("prov_start name=%s", serviceNameBuf);
    break;

  case ARDUINO_EVENT_PROV_CRED_RECV:
    copyToBuf(
        pendingSsid, sizeof(pendingSsid),
        reinterpret_cast<const char *>(event->event_info.prov_cred_recv.ssid));
    copyToBuf(staSsidBuf, sizeof(staSsidBuf), pendingSsid);
    setState(WifiProvisionUiState::Connecting);
    WIFI_LOG("cred_recv ssid='%s'", pendingSsid);
    break;

  case ARDUINO_EVENT_PROV_CRED_FAIL:
    if (event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) {
      provisionFailReason = WifiProvisionFailReason::AuthError;
    } else if (event->event_info.prov_fail_reason ==
               WIFI_PROV_STA_AP_NOT_FOUND) {
      provisionFailReason = WifiProvisionFailReason::ApNotFound;
    } else {
      provisionFailReason = WifiProvisionFailReason::Unknown;
    }

    setState(WifiProvisionUiState::Failed);
    wifi_prov_mgr_reset_sm_state_on_failure();
    WIFI_LOG("cred_fail reason=%d",
             static_cast<int>(event->event_info.prov_fail_reason));
    break;

  case ARDUINO_EVENT_PROV_CRED_SUCCESS:
    setState(WifiProvisionUiState::Connecting);
    WIFI_LOG("cred_success");
    break;

  case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
    IPAddress ip(event->event_info.got_ip.ip_info.ip.addr);
    copyToBuf(staIpBuf, sizeof(staIpBuf), ip.toString().c_str());
    String connectedSsid = WiFi.SSID();
    if (connectedSsid.length() > 0) {
      copyToBuf(staSsidBuf, sizeof(staSsidBuf), connectedSsid.c_str());
    } else if (pendingSsid[0] != '\0') {
      copyToBuf(staSsidBuf, sizeof(staSsidBuf), pendingSsid);
    }
    refreshSavedCredentialsFlag();

    provisioningSessionActive = false;
    setState(WifiProvisionUiState::Connected);
    WIFI_LOG("got_ip ip=%s", staIpBuf);
    break;
  }

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    if (provisionState == WifiProvisionUiState::Connected ||
        provisionState == WifiProvisionUiState::Connecting) {
      setState(WifiProvisionUiState::Connecting);
    }
    WIFI_LOG("sta_disconnected");
    break;

  case ARDUINO_EVENT_PROV_END:
    provisioningSessionActive = false;
    WIFI_LOG("prov_end");
    break;

  default:
    break;
  }
}

void ensureEventHandlerRegistered() {
  if (eventHandlerRegistered)
    return;
  WiFi.onEvent(onProvisionEvent);
  eventHandlerRegistered = true;
}

void clearRuntimeBuffers() {
  std::memset(staSsidBuf, 0, sizeof(staSsidBuf));
  std::memset(staIpBuf, 0, sizeof(staIpBuf));
  std::memset(pendingSsid, 0, sizeof(pendingSsid));
}

bool beginWithSystemCredentials() {
  if (!hasSystemStaCredentials())
    return false;

  WiFi.begin();
  WIFI_LOG("connect_with_system_creds");
  return true;
}

} // namespace

void wifiInitOnBoot() {
  ensureEventHandlerRegistered();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  refreshSavedCredentialsFlag();
  if (hasSavedCredentials) {
    beginWithSystemCredentials();
    WIFI_LOG("boot_connect system_creds");
  } else {
    WIFI_LOG("boot_connect skipped no_credentials");
  }
}

void wifiMaintainConnection() {
  if (currentScreen == SCREEN_WIFI)
    return;
  if (provisioningSessionActive)
    return;
  wl_status_t sta = WiFi.status();
  if (sta == WL_CONNECTED || sta == WL_IDLE_STATUS)
    return;

  unsigned long now = millis();
  if (now - lastReconnectMs < appcfg::WIFI_RECONNECT_INTERVAL_MS)
    return;
  lastReconnectMs = now;

  refreshSavedCredentialsFlag();
  if (!hasSavedCredentials) {
    WIFI_LOG("reconnect_skipped no_credentials");
    return;
  }

  WiFi.mode(WIFI_STA);
  if (WiFi.reconnect()) {
    WIFI_LOG("reconnect_attempt system_creds");
    return;
  }

  WIFI_LOG("reconnect_fallback begin");
  if (beginWithSystemCredentials())
    return;

  WIFI_LOG("reconnect_skipped no_system_creds");
}

void wifiProvisionStart() {
  WIFI_LOG("start_ble");

  clearRuntimeBuffers();
  refreshSavedCredentialsFlag();
  buildServiceName();
  ensureEventHandlerRegistered();

#if CONFIG_BLUEDROID_ENABLED
  provisioningSessionActive = true;

  setState(WifiProvisionUiState::WaitingCredentials);

  WiFiProv.beginProvision(
      WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM,
      WIFI_PROV_SECURITY_1, PROV_POP, serviceNameBuf, nullptr, nullptr, true);
  WiFiProv.printQR(serviceNameBuf, PROV_POP, "ble");
  WIFI_LOG("ble_started name=%s", serviceNameBuf);
#else
  provisioningSessionActive = false;
  setState(WifiProvisionUiState::NotSupported);
  WIFI_LOG("ble_not_supported");
#endif
}

void wifiResetCredentialsAndStartProvisioning() {
  WIFI_LOG("reset_credentials");

#if CONFIG_BLUEDROID_ENABLED
  wifi_prov_mgr_deinit();
#endif

  WiFi.disconnect(true, true);
  clearRuntimeBuffers();
  provisioningSessionActive = false;
  setState(WifiProvisionUiState::Idle);
  refreshSavedCredentialsFlag();
  wifiProvisionStart();
}

void wifiProvisionStop() {
  WIFI_LOG("stop");

#if CONFIG_BLUEDROID_ENABLED
  wifi_prov_mgr_deinit();
#endif

  provisioningSessionActive = false;
  setState(WifiProvisionUiState::Idle);
}

void wifiProvisionUpdate() {
  if (provisionState != WifiProvisionUiState::Connecting)
    return;

  if (WiFi.status() == WL_CONNECTED) {
    if (staIpBuf[0] == '\0') {
      copyToBuf(staIpBuf, sizeof(staIpBuf), WiFi.localIP().toString().c_str());
    }
    if (staSsidBuf[0] == '\0') {
      String connectedSsid = WiFi.SSID();
      copyToBuf(staSsidBuf, sizeof(staSsidBuf), connectedSsid.c_str());
    }
    provisioningSessionActive = false;
    setState(WifiProvisionUiState::Connected);
    return;
  }
}

bool wifiProvisionIsActive() {
  return provisioningSessionActive ||
         provisionState == WifiProvisionUiState::WaitingCredentials ||
         provisionState == WifiProvisionUiState::Connecting;
}

WifiProvisionUiState wifiProvisionState() { return provisionState; }

WifiProvisionFailReason wifiProvisionFailureReason() {
  return provisionFailReason;
}

const char *wifiProvisionApSsid() { return serviceNameBuf; }

const char *wifiProvisionPop() { return PROV_POP; }

const char *wifiProvisionStaSsid() { return staSsidBuf; }

const char *wifiProvisionStaIp() { return staIpBuf; }

bool wifiProvisionLoadSavedCredentials() {
  refreshSavedCredentialsFlag();
  return hasSavedCredentials;
}

bool wifiProvisionHasSavedCredentials() {
  refreshSavedCredentialsFlag();
  return hasSavedCredentials;
}
