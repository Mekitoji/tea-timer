#include <flow/clock_runtime.h>

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sntp.h>

#include <app/app_state.h>
#include <app/clock_time.h>
#include <storage/settings_store.h>
#include <ui.h>

namespace {
#define CLOCK_LOG(fmt, ...) Serial.printf("[clock] " fmt "\n", ##__VA_ARGS__)

constexpr unsigned long CLOCK_NTP_SYNC_TIMEOUT_MS = 15000;
constexpr unsigned long CLOCK_NTP_BOOT_RETRY_INTERVAL_MS = 5000;
constexpr unsigned long CLOCK_NTP_RETRY_INTERVAL_MS = 30000;
constexpr unsigned long CLOCK_NTP_RESYNC_INTERVAL_MS = 21600000;

bool ntpSyncInProgress = false;
bool ntpSyncCompleted = false;
bool ntpLastAttemptFailed = false;
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

unsigned long ntpRetryInterval() {
  return app.clock.timeFreshThisBoot ? CLOCK_NTP_RETRY_INTERVAL_MS
                                     : CLOCK_NTP_BOOT_RETRY_INTERVAL_MS;
}

void scheduleNtpRetry(unsigned long now) {
  ntpNextAttemptMs = now + ntpRetryInterval();
}

void onNtpTimeSynced(timeval *) { ntpSyncCompleted = true; }

bool hasNtpSyncCompleted() {
  return ntpSyncCompleted ||
         sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
}

void startNtpSync() {
  clockConfigureTimezone();
  if (esp_sntp_enabled()) {
    esp_sntp_stop();
  }
  ntpSyncCompleted = false;
  ntpLastAttemptFailed = false;
  sntp_set_time_sync_notification_cb(onNtpTimeSynced);
  sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);
  configTzTime(appcfg::DEFAULT_CLOCK_TZ, "pool.ntp.org", "time.nist.gov");
  ntpSyncInProgress = true;
  ntpSyncStartedMs = millis();
  CLOCK_LOG("ntp_start");
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
  ntpSyncCompleted = false;
  ntpLastAttemptFailed = false;
  ntpNextAttemptMs = 0;
  if (esp_sntp_enabled()) {
    esp_sntp_stop();
  }
}

ClockSyncUiState clockSyncUiState() {
  if (!app.clock.autoSyncEnabled)
    return ClockSyncUiState::Off;
  if (ntpSyncInProgress)
    return ClockSyncUiState::Syncing;
  if (app.clock.timeFreshThisBoot && app.clock.source == ClockSource::Ntp)
    return ClockSyncUiState::Synced;
  if (WiFi.status() != WL_CONNECTED)
    return ClockSyncUiState::WaitingWifi;
  if (ntpLastAttemptFailed)
    return ClockSyncUiState::Failed;
  if (ntpNextAttemptMs != 0 && !hasReachedTime(millis(), ntpNextAttemptMs))
    return ClockSyncUiState::WaitingRetry;
  return ClockSyncUiState::Waiting;
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

  if (hasNtpSyncCompleted()) {
    time_t epoch = 0;
    if (!clockReadSystemEpoch(epoch)) {
      ntpSyncInProgress = false;
      ntpLastAttemptFailed = true;
      scheduleNtpRetry(now);
      CLOCK_LOG("ntp_complete_no_epoch retry_in=%lu", ntpRetryInterval());
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
      ntpLastAttemptFailed = false;

      if (currentScreen == SCREEN_CLOCK && !app.clock.editMode &&
          !hasUnsavedDraft) {
        drawClock();
      }

      ntpNextAttemptMs = now + CLOCK_NTP_RESYNC_INTERVAL_MS;
      CLOCK_LOG("ntp_ok epoch=%llu", static_cast<unsigned long long>(epoch));
    } else {
      ntpLastAttemptFailed = true;
      scheduleNtpRetry(now);
      CLOCK_LOG("ntp_invalid_epoch epoch=%llu retry_in=%lu",
                static_cast<unsigned long long>(epoch), ntpRetryInterval());
    }

    ntpSyncInProgress = false;
    ntpSyncCompleted = false;
    return;
  }

  if (now - ntpSyncStartedMs >= CLOCK_NTP_SYNC_TIMEOUT_MS) {
    if (esp_sntp_enabled()) {
      esp_sntp_stop();
    }
    ntpSyncInProgress = false;
    ntpSyncCompleted = false;
    ntpLastAttemptFailed = true;
    scheduleNtpRetry(now);
    CLOCK_LOG("ntp_timeout retry_in=%lu", ntpRetryInterval());
  }
}

void updateClockScreen() {
  static bool hasLastSyncState = false;
  static ClockSyncUiState lastSyncState = ClockSyncUiState::Waiting;

  if (currentScreen != SCREEN_CLOCK) {
    hasLastSyncState = false;
    return;
  }
  if (app.clock.editMode)
    return;
  if (clockHasUnsavedDraft(app.clock))
    return;

  ClockStateModel previous = app.clock;
  clockRefreshStateFromSystemTime();
  ClockSyncUiState syncState = clockSyncUiState();
  bool syncStateChanged = !hasLastSyncState || lastSyncState != syncState;

  if (previous.year != app.clock.year || previous.month != app.clock.month ||
      previous.day != app.clock.day || previous.hour != app.clock.hour ||
      previous.minute != app.clock.minute || syncStateChanged) {
    drawClock();
  }

  lastSyncState = syncState;
  hasLastSyncState = true;
}
