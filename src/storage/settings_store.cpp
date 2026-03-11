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

BeepProfile clampBeepProfile(int raw) {
  if (raw < static_cast<int>(BeepProfile::Soft))
    return BeepProfile::Soft;
  if (raw > static_cast<int>(BeepProfile::Loud))
    return BeepProfile::Loud;
  return static_cast<BeepProfile>(raw);
}

unsigned long clampDisplayIdleOffMs(unsigned long ms) {
  if (ms < appcfg::MIN_DISPLAY_IDLE_OFF_MS) {
    return appcfg::MIN_DISPLAY_IDLE_OFF_MS;
  } else if (ms > appcfg::MAX_DISPLAY_IDLE_OFF_MS) {
    return appcfg::MAX_DISPLAY_IDLE_OFF_MS;
  }

  return ms;
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

BeepProfile settingsStoreLoadBeepProfile() {
  ensureInitialized();
  int raw = prefs.getInt(appcfg::PREFS_BEEP_PROFILE_KEY,
                         static_cast<int>(appcfg::DEFAULT_BEEP_PROFILE));
  return clampBeepProfile(raw);
}

void settingsStoreSaveBeepProfile(BeepProfile profile) {
  ensureInitialized();
  prefs.putInt(appcfg::PREFS_BEEP_PROFILE_KEY, static_cast<int>(profile));
}

bool settingsStoreLoadAudioEnabled() {
  ensureInitialized();
  return prefs.getBool(appcfg::PREFS_AUDIO_ENABLED_KEY,
                       appcfg::DEFAULT_AUDIO_ENABLED);
}

void settingsStoreSaveAudioEnabled(bool enabled) {
  ensureInitialized();
  prefs.putBool(appcfg::PREFS_AUDIO_ENABLED_KEY, enabled);
}

unsigned long settingsStoreLoadDisplayIdleOffMs() {
  ensureInitialized();
  unsigned long value = prefs.getULong(appcfg::PREFS_DISPLAY_IDLE_OFF_MS_KEY,
                                       appcfg::DEFAULT_DISPLAY_IDLE_OFF_MS);

  return clampDisplayIdleOffMs(value);
}

void settingsStoreSaveDisplayIdleOffMs(unsigned long ms) {
  ensureInitialized();
  prefs.putULong(appcfg::PREFS_DISPLAY_IDLE_OFF_MS_KEY,
                 clampDisplayIdleOffMs(ms));
}
