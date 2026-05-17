#include <flow/device_pairing_flow.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <app/app_config.h>
#include <app/device_identity.h>
#include <app/session_state.h>
#include <app/timer_state.h>
#include <net/api_client.h>
#include <storage/device_auth_store.h>

#include <cstdio>
#include <cstring>

namespace {
DevicePairingFlowState pairingState = DevicePairingFlowState::Idle;
char pairingId[48] = {};
char pairingSecret[128] = {};
char userCode[16] = {};
char verificationUri[160] = {};
char lastMessage[48] = {};
int pollIntervalSec = appcfg::CLOUD_PAIRING_DEFAULT_POLL_MS / 1000;
unsigned long nextPollMs = 0;

void copyCString(char *out, size_t outSize, const char *value) {
  if (!out || outSize == 0)
    return;

  std::strncpy(out, value ? value : "", outSize - 1);
  out[outSize - 1] = '\0';
}

void setMessage(const char *message) {
  copyCString(lastMessage, sizeof(lastMessage), message);
}

void clearPairingRuntime() {
  pairingId[0] = '\0';
  pairingSecret[0] = '\0';
  userCode[0] = '\0';
  verificationUri[0] = '\0';
  pollIntervalSec = appcfg::CLOUD_PAIRING_DEFAULT_POLL_MS / 1000;
  nextPollMs = 0;
}

const char *stateMessage(DevicePairingFlowState state) {
  switch (state) {
  case DevicePairingFlowState::Creating:
    return "Creating";
  case DevicePairingFlowState::Pending:
    return "Waiting for app";
  case DevicePairingFlowState::Authorized:
    return "Paired";
  case DevicePairingFlowState::Expired:
    return "Code expired";
  case DevicePairingFlowState::Consumed:
    return "Already consumed";
  case DevicePairingFlowState::Failed:
    return "Pairing failed";
  case DevicePairingFlowState::Idle:
  default:
    return "Idle";
  }
}

String errorMessageFromResponse(const ApiResponse &response) {
  if (!response.body.isEmpty()) {
    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, response.body) == DeserializationError::Ok) {
      const char *message = doc["error"]["message"] | nullptr;
      if (message && message[0] != '\0')
        return String(message);
    }
  }

  if (!response.error.isEmpty())
    return response.error;

  String message("HTTP ");
  message += response.statusCode;
  return message;
}

void failWithResponse(const ApiResponse &response) {
  String message = errorMessageFromResponse(response);
  copyCString(lastMessage, sizeof(lastMessage), message.c_str());
  pairingState = DevicePairingFlowState::Failed;
}

bool parseCreateResponse(const ApiResponse &response) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response.body);
  if (error) {
    setMessage("Bad pairing response");
    return false;
  }

  const char *id = doc["pairingId"] | "";
  const char *secret = doc["pairingSecret"] | "";
  const char *code = doc["userCode"] | "";
  const char *uri = doc["verificationUri"] | "";
  int interval = doc["pollIntervalSec"] | pollIntervalSec;

  if (id[0] == '\0' || secret[0] == '\0' || code[0] == '\0' ||
      uri[0] == '\0') {
    setMessage("Missing pairing data");
    return false;
  }

  copyCString(pairingId, sizeof(pairingId), id);
  copyCString(pairingSecret, sizeof(pairingSecret), secret);
  copyCString(userCode, sizeof(userCode), code);
  copyCString(verificationUri, sizeof(verificationUri), uri);

  if (interval <= 0)
    interval = appcfg::CLOUD_PAIRING_DEFAULT_POLL_MS / 1000;
  pollIntervalSec = interval;
  nextPollMs = millis() + static_cast<unsigned long>(pollIntervalSec) * 1000UL;
  pairingState = DevicePairingFlowState::Pending;
  setMessage(stateMessage(pairingState));
  return true;
}

void pollPairingStatus() {
  if (pairingId[0] == '\0' || pairingSecret[0] == '\0') {
    pairingState = DevicePairingFlowState::Failed;
    setMessage("Missing pairing secret");
    return;
  }

  char path[96];
  std::snprintf(path, sizeof(path), "/device-pairings/%s/status", pairingId);
  ApiResponse response = apiClientGet(path, pairingSecret);

  if (!response.ok()) {
    failWithResponse(response);
    return;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response.body);
  if (error) {
    pairingState = DevicePairingFlowState::Failed;
    setMessage("Bad status response");
    return;
  }

  const char *status = doc["status"] | "";
  if (std::strcmp(status, "pending") == 0) {
    int interval = doc["pollIntervalSec"] | pollIntervalSec;
    if (interval <= 0)
      interval = appcfg::CLOUD_PAIRING_DEFAULT_POLL_MS / 1000;

    pollIntervalSec = interval;
    nextPollMs =
        millis() + static_cast<unsigned long>(pollIntervalSec) * 1000UL;
    pairingState = DevicePairingFlowState::Pending;
    setMessage(stateMessage(pairingState));
    return;
  }

  if (std::strcmp(status, "expired") == 0) {
    pairingState = DevicePairingFlowState::Expired;
    setMessage(stateMessage(pairingState));
    return;
  }

  if (std::strcmp(status, "consumed") == 0) {
    pairingState = DevicePairingFlowState::Consumed;
    setMessage("Restart pairing");
    return;
  }

  if (std::strcmp(status, "authorized") == 0) {
    const char *deviceId = doc["deviceId"] | "";
    const char *deviceToken = doc["deviceToken"] | "";
    if (deviceId[0] == '\0' || deviceToken[0] == '\0') {
      pairingState = DevicePairingFlowState::Failed;
      setMessage("Missing token");
      return;
    }

    deviceAuthStoreSave(deviceId, deviceToken);
    clearPairingRuntime();
    pairingState = DevicePairingFlowState::Authorized;
    setMessage(stateMessage(pairingState));
    return;
  }

  pairingState = DevicePairingFlowState::Failed;
  setMessage("Unknown status");
}
} // namespace

void devicePairingFlowInit() {
  deviceIdentityInit();
  clearPairingRuntime();
  pairingState = DevicePairingFlowState::Idle;
  setMessage(stateMessage(pairingState));
}

void startDevicePairing() {
  if (WiFi.status() != WL_CONNECTED) {
    pairingState = DevicePairingFlowState::Failed;
    setMessage("WiFi required");
    return;
  }

  DeviceAuthCredentials credentials;
  if (deviceAuthStoreLoad(credentials)) {
    pairingState = DevicePairingFlowState::Authorized;
    setMessage("Already paired");
    return;
  }

  pairingState = DevicePairingFlowState::Creating;
  setMessage(stateMessage(pairingState));

  StaticJsonDocument<384> doc;
  doc["deviceUid"] = deviceIdentityUid();
  doc["name"] = deviceIdentityDefaultName();
  doc["model"] = deviceIdentityModel();
  doc["firmwareVersion"] = deviceIdentityFirmwareVersion();

  String body;
  serializeJson(doc, body);

  ApiResponse response = apiClientPostJson("/device-pairings", body);
  if (!response.ok()) {
    failWithResponse(response);
    return;
  }

  if (!parseCreateResponse(response)) {
    pairingState = DevicePairingFlowState::Failed;
  }
}

void cancelDevicePairing() {
  clearPairingRuntime();
  pairingState = DevicePairingFlowState::Idle;
  setMessage(stateMessage(pairingState));
}

void unpairDevice() {
  deviceAuthStoreClear();
  cancelDevicePairing();
}

void updateDevicePairingFlow() {
  if (pairingState != DevicePairingFlowState::Pending)
    return;
  if (isTimerRunning() || isSessionRunning())
    return;
  if (WiFi.status() != WL_CONNECTED)
    return;
  if (nextPollMs > 0 && millis() < nextPollMs)
    return;

  pollPairingStatus();
}

void devicePairingSnapshot(DevicePairingSnapshot &snapshot) {
  snapshot = DevicePairingSnapshot{};
  copyCString(snapshot.deviceUid, sizeof(snapshot.deviceUid),
              deviceIdentityUid());
  snapshot.state = pairingState;
  copyCString(snapshot.userCode, sizeof(snapshot.userCode), userCode);
  copyCString(snapshot.verificationUri, sizeof(snapshot.verificationUri),
              verificationUri);
  copyCString(snapshot.message, sizeof(snapshot.message), lastMessage);
  snapshot.pollIntervalSec = pollIntervalSec;

  DeviceAuthCredentials credentials;
  if (deviceAuthStoreLoad(credentials)) {
    snapshot.paired = true;
    copyCString(snapshot.deviceId, sizeof(snapshot.deviceId),
                credentials.deviceId);
  }
}
