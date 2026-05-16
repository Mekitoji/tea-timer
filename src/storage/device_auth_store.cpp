#include <storage/device_auth_store.h>

#include <Arduino.h>
#include <Preferences.h>
#include <app/app_config.h>

namespace {
Preferences prefs;
bool initialized = false;

void ensureInitialized() {
  if (initialized)
    return;

  prefs.begin(appcfg::DEVICE_AUTH_PREFS_NAMESPACE, false);
  initialized = true;
}

void copyStringToBuffer(const String &value, char *out, size_t outSize) {
  if (!out || outSize == 0)
    return;

  value.toCharArray(out, outSize);
}
} // namespace

void deviceAuthStoreBegin() { ensureInitialized(); }

bool deviceAuthStoreLoad(DeviceAuthCredentials &credentials) {
  ensureInitialized();

  credentials = DeviceAuthCredentials{};
  String deviceId = prefs.getString(appcfg::PREFS_DEVICE_ID_KEY, "");
  String deviceToken = prefs.getString(appcfg::PREFS_DEVICE_TOKEN_KEY, "");

  if (deviceId.isEmpty() || deviceToken.isEmpty())
    return false;

  copyStringToBuffer(deviceId, credentials.deviceId,
                     sizeof(credentials.deviceId));
  copyStringToBuffer(deviceToken, credentials.deviceToken,
                     sizeof(credentials.deviceToken));
  credentials.paired = true;
  return true;
}

void deviceAuthStoreSave(const char *deviceId, const char *deviceToken) {
  ensureInitialized();

  prefs.putString(appcfg::PREFS_DEVICE_ID_KEY, deviceId ? deviceId : "");
  prefs.putString(appcfg::PREFS_DEVICE_TOKEN_KEY,
                  deviceToken ? deviceToken : "");
}

void deviceAuthStoreClear() {
  ensureInitialized();

  prefs.remove(appcfg::PREFS_DEVICE_ID_KEY);
  prefs.remove(appcfg::PREFS_DEVICE_TOKEN_KEY);
}
