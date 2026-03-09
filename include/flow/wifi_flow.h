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

void wifiProvisionStart();
void wifiProvisionStop();
void wifiProvisionUpdate();
void wifiResetCredentialsAndStartProvisioning();
void wifiInitOnBoot();
void wifiMaintainConnection();

bool wifiProvisionIsActive();
WifiProvisionUiState wifiProvisionState();

WifiProvisionFailReason wifiProvisionFailureReason();
const char *wifiProvisionApSsid();
const char *wifiProvisionPop();
const char *wifiProvisionStaSsid();
const char *wifiProvisionStaIp();

bool wifiProvisionLoadSavedCredentials();
bool wifiProvisionHasSavedCredentials();
