#include <flow/clock_runtime.h>

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sntp.h>

#include <app/app_state.h>
#include <app/clock_time.h>
#include <storage/settings_store.h>
#include <ui.h>

namespace {
constexpr unsigned long CLOCK_NTP_SYNC_TIMEOUT_MS = 15000;
constexpr unsigned long CLOCK_NTP_RETRY_INTERVAL_MS = 30000;
constexpr unsigned long CLOCK_NTP_RESYNC_INTERVAL_MS = 21600000;

bool ntpSyncInProgress = false;
unsigned long ntpSyncStartedMs = 0;
unsigned long ntpNextAttemptMs = 0;

bool clockReadSystemEpoch(time_t &outEpoch) {
  time_t epoch = std::time(nullptr);
  if (epoch <= 0)
    return false;

  outEpoch = epoch;
  return true;
}

bool isAutoSyncReady() {
  return app.clock.autoSyncEnabled && WiFi.status() == WL_CONNECTED;
}

bool hasReachedTime(unsigned long now, unsigned long target) {
  return static_cast<long>(now - target) >= 0;
}

void startNtpSync() {
  clockConfigureTimezone();
  configTzTime(appcfg::DEFAULT_CLOCK_TZ, "pool.ntp.org", "time.nist.gov");
  sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);
  ntpSyncInProgress = true;
  ntpSyncStartedMs = millis();
}
} // namespace

void clockPersistState(time_t epoch) {
  settingsStoreSaveClockEpoch(static_cast<unsigned long long>(epoch));
  settingsStoreSaveClockAutoSyncEnabled(app.clock.autoSyncEnabled);
  settingsStoreSaveClockSource(static_cast<uint8_t>(app.clock.source));
  settingsStoreSaveClockValid(app.clock.timeValid);
}

void clockCancelNtpSync() {
  ntpSyncInProgress = false;
  ntpNextAttemptMs = 0;
  if (esp_sntp_enabled()) {
    esp_sntp_stop();
  }
}

void clockRefreshStateFromSystemTime() {
  time_t epoch = 0;
  if (!clockReadSystemEpoch(epoch))
    return;

  clockApplyEpochToState(app.clock, epoch);
  clockCopyStateToDraft(app.clock);
}

void clockInitializeRuntime(unsigned long long savedEpoch) {
  clockConfigureTimezone();

  bool initializedFromSavedEpoch = false;
  app.clock.timeFreshThisBoot = false;

  if (app.clock.timeValid && savedEpoch > 0) {
    time_t epoch = static_cast<time_t>(savedEpoch);
    if (clockSetSystemTimeFromEpoch(epoch)) {
      clockApplyEpochToState(app.clock, epoch);
      initializedFromSavedEpoch = true;
    }
  }

  if (!initializedFromSavedEpoch) {
    time_t buildEpoch = 0;
    if (clockBuildEpochFromCompileTime(buildEpoch)) {
      clockSetSystemTimeFromEpoch(buildEpoch);
      clockApplyEpochToState(app.clock, buildEpoch);
    } else {
      app.clock.year = 2026;
      app.clock.month = 1;
      app.clock.day = 1;
      app.clock.hour = 0;
      app.clock.minute = 0;
    }

    app.clock.timeValid = false;
    app.clock.source = ClockSource::Default;
  }

  clockCopyStateToDraft(app.clock);
}

void clockRequestNtpSync() {
  if (!isAutoSyncReady())
    return;

  ntpNextAttemptMs = 0;
  if (!ntpSyncInProgress) {
    startNtpSync();
  }
}

void updateClockRuntime() {
  if (!app.clock.autoSyncEnabled) {
    clockCancelNtpSync();
    return;
  }

  unsigned long now = millis();

  if (!ntpSyncInProgress) {
    if (isAutoSyncReady() && hasReachedTime(now, ntpNextAttemptMs)) {
      startNtpSync();
    }
    return;
  }

  if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
    time_t epoch = 0;
    if (!clockReadSystemEpoch(epoch)) {
      ntpSyncInProgress = false;
      ntpNextAttemptMs = now + CLOCK_NTP_RETRY_INTERVAL_MS;
      return;
    }

    if (epoch > 1700000000) {
      bool hasUnsavedDraft = clockHasUnsavedDraft(app.clock);

      app.clock.source = ClockSource::Ntp;
      app.clock.timeValid = true;
      app.clock.timeFreshThisBoot = true;
      if (!hasUnsavedDraft) {
        clockApplyEpochToState(app.clock, epoch);
        clockCopyStateToDraft(app.clock);
      }
      clockPersistState(epoch);

      if (currentScreen == SCREEN_CLOCK && !app.clock.editMode &&
          !hasUnsavedDraft) {
        drawClock();
      }

      ntpNextAttemptMs = now + CLOCK_NTP_RESYNC_INTERVAL_MS;
    } else {
      ntpNextAttemptMs = now + CLOCK_NTP_RETRY_INTERVAL_MS;
    }

    ntpSyncInProgress = false;
    return;
  }

  if (now - ntpSyncStartedMs >= CLOCK_NTP_SYNC_TIMEOUT_MS) {
    if (esp_sntp_enabled()) {
      esp_sntp_stop();
    }
    ntpSyncInProgress = false;
    ntpNextAttemptMs = now + CLOCK_NTP_RETRY_INTERVAL_MS;
  }
}

void updateClockScreen() {
  if (currentScreen != SCREEN_CLOCK)
    return;
  if (app.clock.editMode)
    return;
  if (clockHasUnsavedDraft(app.clock))
    return;

  ClockStateModel previous = app.clock;
  clockRefreshStateFromSystemTime();

  if (previous.year != app.clock.year || previous.month != app.clock.month ||
      previous.day != app.clock.day || previous.hour != app.clock.hour ||
      previous.minute != app.clock.minute) {
    drawClock();
  }
}
