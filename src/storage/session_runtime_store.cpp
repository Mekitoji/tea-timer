#include <storage/session_runtime_store.h>

#include <Preferences.h>

#include <app/app_config.h>

namespace {
Preferences prefs;
bool initialized = false;

void ensureInitialized() {
  if (initialized)
    return;
  prefs.begin(appcfg::PREFS_NAMESPACE, false);
  initialized = true;
}
} // namespace

bool sessionRuntimeStoreLoad(SessionRuntimeSnapshot &snapshot) {
  ensureInitialized();

  snapshot = SessionRuntimeSnapshot{};
  size_t bytes = prefs.getBytesLength(appcfg::PREFS_SESSION_RUNTIME_SNAPSHOT_KEY);
  if (bytes != sizeof(SessionRuntimeSnapshot)) {
    if (bytes > 0)
      prefs.remove(appcfg::PREFS_SESSION_RUNTIME_SNAPSHOT_KEY);
    return false;
  }

  size_t read = prefs.getBytes(appcfg::PREFS_SESSION_RUNTIME_SNAPSHOT_KEY,
                               &snapshot, sizeof(SessionRuntimeSnapshot));
  if (read != sizeof(SessionRuntimeSnapshot)) {
    snapshot = SessionRuntimeSnapshot{};
    prefs.remove(appcfg::PREFS_SESSION_RUNTIME_SNAPSHOT_KEY);
    return false;
  }

  if (snapshot.version != appcfg::SESSION_RUNTIME_SNAPSHOT_VERSION ||
      !snapshot.valid) {
    snapshot = SessionRuntimeSnapshot{};
    prefs.remove(appcfg::PREFS_SESSION_RUNTIME_SNAPSHOT_KEY);
    return false;
  }

  return true;
}

void sessionRuntimeStoreSave(const SessionRuntimeSnapshot &snapshot) {
  ensureInitialized();
  prefs.putBytes(appcfg::PREFS_SESSION_RUNTIME_SNAPSHOT_KEY, &snapshot,
                 sizeof(SessionRuntimeSnapshot));
}

void sessionRuntimeStoreClear() {
  ensureInitialized();
  prefs.remove(appcfg::PREFS_SESSION_RUNTIME_SNAPSHOT_KEY);
}
