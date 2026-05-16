#pragma once

struct DeviceAuthCredentials {
  bool paired = false;
  char deviceId[48] = {};
  char deviceToken[128] = {};
};

void deviceAuthStoreBegin();
bool deviceAuthStoreLoad(DeviceAuthCredentials &credentials);
void deviceAuthStoreSave(const char *deviceId, const char *deviceToken);
void deviceAuthStoreClear();
