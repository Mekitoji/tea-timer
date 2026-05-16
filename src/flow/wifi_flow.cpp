#include <flow/wifi_flow.h>

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiProv.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <flow/clock_runtime.h>
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
unsigned long connectAttemptStartedMs = 0;

bool beginWithSystemCredentials();
void refreshSavedCredentialsFlag();

void copyToBuf(char *dst, size_t dstSize, const char *src) {
  if (!dst || dstSize == 0)
    return;
  if (!src) {
    dst[0] = '\0';
    return;
  }
  std::snprintf(dst, dstSize, "%s", src);
}

const char *disconnectReasonName(uint8_t reason) {
  switch (reason) {
  case WIFI_REASON_AUTH_EXPIRE:
    return "AUTH_EXPIRE";
  case WIFI_REASON_TIMEOUT:
    return "TIMEOUT";
  case WIFI_REASON_BEACON_TIMEOUT:
    return "BEACON_TIMEOUT";
  case WIFI_REASON_NO_AP_FOUND:
    return "NO_AP_FOUND";
  case WIFI_REASON_AUTH_FAIL:
    return "AUTH_FAIL";
  case WIFI_REASON_ASSOC_FAIL:
    return "ASSOC_FAIL";
  case WIFI_REASON_HANDSHAKE_TIMEOUT:
    return "HANDSHAKE_TIMEOUT";
  case WIFI_REASON_CONNECTION_FAIL:
    return "CONNECTION_FAIL";
  default:
    return "OTHER";
  }
}

void setState(WifiProvisionUiState next) {
  provisionState = next;

  if (provisionState != WifiProvisionUiState::Failed) {
    provisionFailReason = WifiProvisionFailReason::None;
  }

  WIFI_LOG("state=%d", static_cast<int>(provisionState));
}

void markConnectAttemptStarted() { connectAttemptStartedMs = millis(); }

void clearConnectAttempt() { connectAttemptStartedMs = 0; }

bool connectAttemptTimedOut(unsigned long now) {
  return connectAttemptStartedMs != 0 &&
         now - connectAttemptStartedMs >= appcfg::WIFI_CONNECT_TIMEOUT_MS;
}

bool provisionStateIsActive() {
  return provisionState == WifiProvisionUiState::WaitingCredentials ||
         provisionState == WifiProvisionUiState::Connecting ||
         provisionState == WifiProvisionUiState::Failed;
}

bool provisioningBlocksStaReconnect() {
  return provisioningSessionActive ||
         provisionState == WifiProvisionUiState::WaitingCredentials ||
         provisionState == WifiProvisionUiState::Failed ||
         provisionState == WifiProvisionUiState::NotSupported;
}

bool shouldStopProvisioningOnScreenExit() {
  refreshSavedCredentialsFlag();
  return provisioningBlocksStaReconnect() ||
         (!hasSavedCredentials &&
          provisionState == WifiProvisionUiState::Connecting);
}

WifiStaUiState mapStaStatus(wl_status_t status) {
  switch (status) {
  case WL_CONNECTED:
    return WifiStaUiState::Connected;
  case WL_CONNECT_FAILED:
    return WifiStaUiState::ConnectFailed;
  case WL_CONNECTION_LOST:
    return WifiStaUiState::ConnectionLost;
  case WL_NO_SSID_AVAIL:
    return WifiStaUiState::NoSsid;
  case WL_IDLE_STATUS:
    return WifiStaUiState::Connecting;
  case WL_DISCONNECTED:
    return WifiStaUiState::Disconnected;
  default:
    return WifiStaUiState::Unknown;
  }
}

void enterWaitingCredentials() {
  provisioningSessionActive = true;
  setState(WifiProvisionUiState::WaitingCredentials);
}

WifiProvisionFailReason mapProvisionFailReason(
    wifi_prov_sta_fail_reason_t reason) {
  switch (reason) {
  case WIFI_PROV_STA_AUTH_ERROR:
    return WifiProvisionFailReason::AuthError;
  case WIFI_PROV_STA_AP_NOT_FOUND:
    return WifiProvisionFailReason::ApNotFound;
  default:
    return WifiProvisionFailReason::Unknown;
  }
}

bool hasSystemStaCredentials() {
  wifi_config_t conf = {};

  if (esp_wifi_get_config(WIFI_IF_STA, &conf) != ESP_OK)
    return false;

  return conf.sta.ssid[0] != '\0';
}

bool loadSystemStaCredentials(wifi_config_t &conf) {
  std::memset(&conf, 0, sizeof(conf));
  if (esp_wifi_get_config(WIFI_IF_STA, &conf) != ESP_OK)
    return false;

  if (conf.sta.ssid[0] == '\0')
    return false;

  copyToBuf(staSsidBuf, sizeof(staSsidBuf),
            reinterpret_cast<const char *>(conf.sta.ssid));
  return true;
}

void refreshSavedCredentialsFlag() {
  hasSavedCredentials = hasSystemStaCredentials();
}

void resetStoredWifiConfig() {
  WiFi.mode(WIFI_STA);
  bool erased = WiFi.eraseAP();
  bool disconnected = WiFi.disconnect(true, true);
  hasSavedCredentials = false;
  clearConnectAttempt();
  WIFI_LOG("wifi_config_reset erase=%d disconnect=%d", erased ? 1 : 0,
           disconnected ? 1 : 0);
}

void buildServiceName() {
  uint64_t chipId = ESP.getEfuseMac();
  uint8_t b3 = static_cast<uint8_t>((chipId >> 16) & 0xFF);
  uint8_t b4 = static_cast<uint8_t>((chipId >> 8) & 0xFF);
  uint8_t b5 = static_cast<uint8_t>(chipId & 0xFF);
  std::snprintf(serviceNameBuf, sizeof(serviceNameBuf), "PROV_%02X%02X%02X", b3,
                b4, b5);
}

void cacheConnectedStaInfo(IPAddress ip, bool onlyMissing) {
  if ((!onlyMissing || staIpBuf[0] == '\0') &&
      ip != IPAddress(static_cast<uint32_t>(0))) {
    copyToBuf(staIpBuf, sizeof(staIpBuf), ip.toString().c_str());
  }

  if (onlyMissing && staSsidBuf[0] != '\0')
    return;

  String connectedSsid = WiFi.SSID();
  if (connectedSsid.length() > 0) {
    copyToBuf(staSsidBuf, sizeof(staSsidBuf), connectedSsid.c_str());
  } else if (pendingSsid[0] != '\0') {
    copyToBuf(staSsidBuf, sizeof(staSsidBuf), pendingSsid);
  }
}

void onProvisionEvent(arduino_event_t *event) {
  if (!event)
    return;

  switch (event->event_id) {
  case ARDUINO_EVENT_PROV_START:
    enterWaitingCredentials();
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
    provisioningSessionActive = false;
    clearConnectAttempt();
    provisionFailReason =
        mapProvisionFailReason(event->event_info.prov_fail_reason);
    setState(WifiProvisionUiState::Failed);
    wifi_prov_mgr_reset_sm_state_on_failure();
    WIFI_LOG("cred_fail reason=%d",
             static_cast<int>(event->event_info.prov_fail_reason));
    break;

  case ARDUINO_EVENT_PROV_CRED_SUCCESS:
    provisioningSessionActive = false;
    refreshSavedCredentialsFlag();
    setState(WifiProvisionUiState::Connecting);
    WIFI_LOG("cred_success saved=%d", hasSavedCredentials ? 1 : 0);
    if (hasSavedCredentials) {
      beginWithSystemCredentials();
    }
    break;

  case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
    IPAddress ip(event->event_info.got_ip.ip_info.ip.addr);
    cacheConnectedStaInfo(ip, false);
    refreshSavedCredentialsFlag();

    provisioningSessionActive = false;
    clearConnectAttempt();
    if (app.clock.autoSyncEnabled) {
      clockRequestNtpSync();
    }
    setState(WifiProvisionUiState::Connected);
    WIFI_LOG("got_ip ip=%s", staIpBuf);
    break;
  }

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {
    if (provisionState == WifiProvisionUiState::Connected ||
        provisionState == WifiProvisionUiState::Connecting) {
      setState(WifiProvisionUiState::Connecting);
    }
    const uint8_t reason = event->event_info.wifi_sta_disconnected.reason;
    WIFI_LOG("sta_disconnected reason=%d %s", static_cast<int>(reason),
             disconnectReasonName(reason));
    break;
  }

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
  wifi_config_t conf = {};
  if (!loadSystemStaCredentials(conf))
    return false;

  WiFi.mode(WIFI_STA);
  if (esp_wifi_set_config(WIFI_IF_STA, &conf) != ESP_OK) {
    WIFI_LOG("connect_with_system_creds set_config_failed");
    return false;
  }

  setState(WifiProvisionUiState::Connecting);
  wl_status_t status = WiFi.begin();
  markConnectAttemptStarted();
  WIFI_LOG("connect_with_system_creds ssid='%s' status=%d", staSsidBuf,
           static_cast<int>(status));
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
  if (provisioningBlocksStaReconnect())
    return;
  wl_status_t sta = WiFi.status();
  if (sta == WL_CONNECTED) {
    clearConnectAttempt();
    return;
  }

  unsigned long now = millis();
  bool timedOut = connectAttemptTimedOut(now);
  if (sta == WL_IDLE_STATUS && !timedOut)
    return;

  if (!timedOut && now - lastReconnectMs < appcfg::WIFI_RECONNECT_INTERVAL_MS)
    return;
  lastReconnectMs = now;

  refreshSavedCredentialsFlag();
  if (!hasSavedCredentials) {
    WIFI_LOG("reconnect_skipped no_credentials");
    return;
  }

  if (timedOut) {
    WIFI_LOG("connect_timeout status=%d elapsed=%lu", static_cast<int>(sta),
             now - connectAttemptStartedMs);
  }

  if (connectAttemptStartedMs != 0) {
    WiFi.disconnect(false, false);
  }

  WIFI_LOG("reconnect_attempt system_creds");
  if (beginWithSystemCredentials()) {
    return;
  }

  WIFI_LOG("reconnect_skipped no_system_creds");
}

static void wifiProvisionStart() {
  WIFI_LOG("start_ble");

  clearConnectAttempt();
  clearRuntimeBuffers();
  refreshSavedCredentialsFlag();
  buildServiceName();
  ensureEventHandlerRegistered();

#if CONFIG_BLUEDROID_ENABLED
  enterWaitingCredentials();

  WiFiProv.beginProvision(
      WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_NONE,
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

  resetStoredWifiConfig();
  clearRuntimeBuffers();
  provisioningSessionActive = false;
  setState(WifiProvisionUiState::Idle);
  refreshSavedCredentialsFlag();
  wifiProvisionStart();
}

static void wifiProvisionStop() {
  if (!shouldStopProvisioningOnScreenExit())
    return;

  WIFI_LOG("stop");

#if CONFIG_BLUEDROID_ENABLED
  wifi_prov_mgr_deinit();
#endif

  provisioningSessionActive = false;
  setState(WifiProvisionUiState::Idle);
}

static void wifiProvisionUpdate() {
  if (provisionState != WifiProvisionUiState::Connecting)
    return;

  if (WiFi.status() == WL_CONNECTED) {
    cacheConnectedStaInfo(WiFi.localIP(), true);
    provisioningSessionActive = false;
    setState(WifiProvisionUiState::Connected);
    return;
  }
}

static bool ensureProvisioningStartedForScreen() {
  refreshSavedCredentialsFlag();
  if (!hasSavedCredentials && !wifiProvisionIsActive()) {
    wifiProvisionStart();
  }
  return hasSavedCredentials;
}

void wifiFlowEnterScreen() { ensureProvisioningStartedForScreen(); }

void wifiFlowExitScreen() { wifiProvisionStop(); }

void wifiFlowTick() {
  bool hasSaved = ensureProvisioningStartedForScreen();
  if (!hasSaved) {
    wifiProvisionUpdate();
  }
}

WifiFlowSnapshot wifiFlowSnapshot() {
  refreshSavedCredentialsFlag();

  wl_status_t sta = WiFi.status();
  if (sta == WL_CONNECTED) {
    cacheConnectedStaInfo(WiFi.localIP(), false);
  }

  WifiFlowSnapshot snapshot;
  snapshot.hasSavedCredentials = hasSavedCredentials;
  snapshot.setupMode = !hasSavedCredentials;
  snapshot.connected =
      sta == WL_CONNECTED || provisionState == WifiProvisionUiState::Connected;
  snapshot.rssi = snapshot.connected ? WiFi.RSSI() : 0;
  if (snapshot.connected) {
    snapshot.staState = WifiStaUiState::Connected;
  } else if (connectAttemptStartedMs != 0) {
    snapshot.staState = WifiStaUiState::Connecting;
  } else {
    snapshot.staState = mapStaStatus(sta);
  }
  snapshot.provisionState = provisionState;
  snapshot.provisionFailReason = provisionFailReason;
  snapshot.serviceName = serviceNameBuf;
  snapshot.pop = PROV_POP;
  snapshot.staSsid = staSsidBuf;
  snapshot.staIp = staIpBuf;
  return snapshot;
}

bool wifiProvisionIsActive() {
  return provisioningSessionActive || provisionStateIsActive();
}

void wifiRetryFailedProvisioning() {
  if (provisionState != WifiProvisionUiState::Failed) {
    return;
  }

  clearRuntimeBuffers();
  enterWaitingCredentials();
}

WifiProvisionUiState wifiProvisionState() { return provisionState; }

WifiProvisionFailReason wifiProvisionFailureReason() {
  return provisionFailReason;
}

const char *wifiProvisionApSsid() { return serviceNameBuf; }

const char *wifiProvisionPop() { return PROV_POP; }

const char *wifiProvisionStaSsid() { return staSsidBuf; }

const char *wifiProvisionStaIp() { return staIpBuf; }

bool wifiProvisionHasSavedCredentials() {
  refreshSavedCredentialsFlag();
  return hasSavedCredentials;
}
