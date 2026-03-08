#pragma once

enum class WifiProvisionUiState {
  Idle,
  WaitingCredentials,
  Connecting,
  Connected,
  Failed,
  NotSupported,
};

void wifiProvisionStart();
void wifiProvisionStop();
void wifiProvisionUpdate();
void wifiResetCredentialsAndStartProvisioning();
void wifiInitOnBoot();
void wifiMaintainConnection();

bool wifiProvisionIsActive();
WifiProvisionUiState wifiProvisionState();

const char *wifiProvisionStatusText();
const char *wifiProvisionFailureReasonText();
const char *wifiProvisionApSsid();
const char *wifiProvisionPop();
const char *wifiProvisionStaSsid();
const char *wifiProvisionStaIp();

bool wifiProvisionLoadSavedCredentials();
bool wifiProvisionHasSavedCredentials();
