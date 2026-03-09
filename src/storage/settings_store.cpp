#include <storage/settings_store.h>

#include <Preferences.h>
#include <app/app_config.h>
#include <app/tea_config.h>

namespace {
Preferences prefs;
bool initialized = false;

void ensureInitialized() {
  if (initialized)
    return;
  prefs.begin(appcfg::PREFS_NAMESPACE, false);
  initialized = true;
}

int clampTimerDuration(int seconds) {
  if (seconds < MIN_TIME)
    return MIN_TIME;
  if (seconds > MAX_TIME)
    return MAX_TIME;
  return seconds;
}
} // namespace

void settingsStoreBegin() { ensureInitialized(); }

int settingsStoreLoadTimerDurationSec() {
  ensureInitialized();
  int value =
      prefs.getInt(appcfg::PREFS_DURATION_KEY, appcfg::DEFAULT_TIMER_DURATION);
  return clampTimerDuration(value);
}

void settingsStoreSaveTimerDurationSec(int seconds) {
  ensureInitialized();
  prefs.putInt(appcfg::PREFS_DURATION_KEY, clampTimerDuration(seconds));
}

bool settingsStoreLoadPowerSaveEnabled() {
  ensureInitialized();
  return prefs.getBool(appcfg::PREFS_POWER_SAVE_KEY,
                       appcfg::DEFAULT_POWER_SAVE_ENABLED);
}

void settingsStoreSavePowerSaveEnabled(bool enabled) {
  ensureInitialized();
  prefs.putBool(appcfg::PREFS_POWER_SAVE_KEY, enabled);
}
