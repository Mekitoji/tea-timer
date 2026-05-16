#include <app/device_identity.h>

#include <Arduino.h>
#include <app/app_config.h>
#include <cstdio>

namespace {
char uid[24] = {};
bool initialized = false;
} // namespace

void deviceIdentityInit() {
  if (initialized)
    return;

  uint64_t mac = ESP.getEfuseMac();
  std::snprintf(uid, sizeof(uid), "esp32c3-%012llx",
                static_cast<unsigned long long>(mac));
  initialized = true;
}

const char *deviceIdentityUid() {
  deviceIdentityInit();
  return uid;
}

const char *deviceIdentityDefaultName() { return appcfg::DEVICE_DEFAULT_NAME; }

const char *deviceIdentityModel() { return appcfg::DEVICE_MODEL; }

const char *deviceIdentityFirmwareVersion() {
  return appcfg::FIRMWARE_VERSION;
}
