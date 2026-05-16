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

void wifiFlowEnterScreen();
void wifiFlowExitScreen();
void wifiFlowTick();
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
