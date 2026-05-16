#pragma once

#include <cstdint>

enum class DevicePairingFlowState : uint8_t {
  Idle = 0,
  Creating = 1,
  Pending = 2,
  Authorized = 3,
  Expired = 4,
  Consumed = 5,
  Failed = 6
};

struct DevicePairingSnapshot {
  bool paired = false;
  char deviceUid[24] = {};
  char deviceId[48] = {};
  DevicePairingFlowState state = DevicePairingFlowState::Idle;
  char userCode[16] = {};
  char message[48] = {};
  int pollIntervalSec = 0;
};

void devicePairingFlowInit();
void startDevicePairing();
void cancelDevicePairing();
void unpairDevice();
void updateDevicePairingFlow();
void devicePairingSnapshot(DevicePairingSnapshot &snapshot);
