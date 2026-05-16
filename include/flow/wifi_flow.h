#pragma once

enum class WifiProvisionUiState {
  Idle,
  WaitingCredentials,
  Connecting,
  Connected,
  Failed,
  NotSupported,
};

enum class WifiProvisionFailReason {
  None,
  AuthError,
  ApNotFound,
  Unknown,
};

enum class WifiStaUiState {
  Disconnected,
  Connecting,
  Connected,
  ConnectFailed,
  ConnectionLost,
  NoSsid,
  Unknown,
};

struct WifiFlowSnapshot {
  bool hasSavedCredentials = false;
  bool setupMode = true;
  bool connected = false;
  int rssi = 0;
  WifiStaUiState staState = WifiStaUiState::Disconnected;
  WifiProvisionUiState provisionState = WifiProvisionUiState::Idle;
  WifiProvisionFailReason provisionFailReason = WifiProvisionFailReason::None;
  const char *serviceName = "";
  const char *pop = "";
  const char *staSsid = "";
  const char *staIp = "";
};

void wifiFlowEnterScreen();
void wifiFlowExitScreen();
void wifiFlowTick();
WifiFlowSnapshot wifiFlowSnapshot();
void wifiResetCredentialsAndStartProvisioning();
void wifiInitOnBoot();
void wifiMaintainConnection();
void wifiRetryFailedProvisioning();

bool wifiProvisionIsActive();
WifiProvisionUiState wifiProvisionState();

WifiProvisionFailReason wifiProvisionFailureReason();
const char *wifiProvisionApSsid();
const char *wifiProvisionPop();
const char *wifiProvisionStaSsid();
const char *wifiProvisionStaIp();

bool wifiProvisionHasSavedCredentials();
