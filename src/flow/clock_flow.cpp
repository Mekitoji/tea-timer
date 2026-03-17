#include <flow/clock_flow.h>

#include <Arduino.h>
#include <WiFi.h>
#include <cstring>
#include <ctime>
#include <esp_sntp.h>
#include <sys/time.h>

#include <app/app_config.h>
#include <app/app_state.h>
#include <flow/navigation_flow.h>
#include <storage/settings_store.h>
#include <ui.h>

namespace {
constexpr unsigned long CLOCK_NTP_SYNC_TIMEOUT_MS = 15000;
constexpr unsigned long CLOCK_NTP_RETRY_INTERVAL_MS = 30000;
constexpr unsigned long CLOCK_NTP_RESYNC_INTERVAL_MS = 21600000;

bool ntpSyncInProgress = false;
bool ntpCallbackRegistered = false;
bool ntpSyncCompletedPending = false;
unsigned long ntpSyncStartedMs = 0;
unsigned long ntpNextAttemptMs = 0;
portMUX_TYPE ntpSyncMux = portMUX_INITIALIZER_UNLOCKED;

void onSntpTimeSync(struct timeval *tv) {
  (void)tv;
  portENTER_CRITICAL(&ntpSyncMux);
  ntpSyncCompletedPending = true;
  portEXIT_CRITICAL(&ntpSyncMux);
}

void configureClockTimezone() {
  setenv("TZ", appcfg::DEFAULT_CLOCK_TZ, 1);
  tzset();
}

ClockRow nextRow(ClockRow row, bool plus) {
  if (plus) {
    if (row == ClockRow::Time)
      return ClockRow::Date;
    if (row == ClockRow::Date)
      return ClockRow::AutoSync;
    return ClockRow::Time;
  }

  if (row == ClockRow::AutoSync)
    return ClockRow::Date;
  if (row == ClockRow::Date)
    return ClockRow::Time;
  return ClockRow::AutoSync;
}

int daysInMonth(int year, int month) {
  if (month == 2) {
    bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    return leap ? 29 : 28;
  }
  if (month == 4 || month == 6 || month == 9 || month == 11)
    return 30;
  return 31;
}

void clampDraftDate() {
  if (app.clock.draftMonth < 1)
    app.clock.draftMonth = 12;
  if (app.clock.draftMonth > 12)
    app.clock.draftMonth = 1;

  if (app.clock.draftYear < 2024)
    app.clock.draftYear = 2024;
  if (app.clock.draftYear > 2099)
    app.clock.draftYear = 2099;

  int maxDay = daysInMonth(app.clock.draftYear, app.clock.draftMonth);
  if (app.clock.draftDay < 1)
    app.clock.draftDay = maxDay;
  if (app.clock.draftDay > maxDay)
    app.clock.draftDay = 1;
}

void copyClockStateToDraft() {
  app.clock.draftAutoSyncEnabled = app.clock.autoSyncEnabled;
  app.clock.draftYear = app.clock.year;
  app.clock.draftMonth = app.clock.month;
  app.clock.draftDay = app.clock.day;
  app.clock.draftHour = app.clock.hour;
  app.clock.draftMinute = app.clock.minute;
}

int monthFromAbbrev(const char *month) {
  if (std::strcmp(month, "Jan") == 0)
    return 1;
  if (std::strcmp(month, "Feb") == 0)
    return 2;
  if (std::strcmp(month, "Mar") == 0)
    return 3;
  if (std::strcmp(month, "Apr") == 0)
    return 4;
  if (std::strcmp(month, "May") == 0)
    return 5;
  if (std::strcmp(month, "Jun") == 0)
    return 6;
  if (std::strcmp(month, "Jul") == 0)
    return 7;
  if (std::strcmp(month, "Aug") == 0)
    return 8;
  if (std::strcmp(month, "Sep") == 0)
    return 9;
  if (std::strcmp(month, "Oct") == 0)
    return 10;
  if (std::strcmp(month, "Nov") == 0)
    return 11;
  if (std::strcmp(month, "Dec") == 0)
    return 12;
  return 1;
}

bool buildEpochFromCompileTime(time_t &outEpoch) {
  char monthBuf[4] = {};
  int day = 1;
  int year = 2026;
  int hour = 0;
  int minute = 0;
  int second = 0;

  if (std::sscanf(__DATE__, "%3s %d %d", monthBuf, &day, &year) != 3)
    return false;
  if (std::sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second) != 3)
    return false;

  std::tm tmValue = {};
  tmValue.tm_year = year - 1900;
  tmValue.tm_mon = monthFromAbbrev(monthBuf) - 1;
  tmValue.tm_mday = day;
  tmValue.tm_hour = hour;
  tmValue.tm_min = minute;
  tmValue.tm_sec = second;
  tmValue.tm_isdst = -1;

  time_t epoch = std::mktime(&tmValue);
  if (epoch < 0)
    return false;

  outEpoch = epoch;
  return true;
}

bool buildEpochFromDraft(time_t &outEpoch) {
  std::tm tmValue = {};
  tmValue.tm_year = app.clock.draftYear - 1900;
  tmValue.tm_mon = app.clock.draftMonth - 1;
  tmValue.tm_mday = app.clock.draftDay;
  tmValue.tm_hour = app.clock.draftHour;
  tmValue.tm_min = app.clock.draftMinute;
  tmValue.tm_sec = 0;
  tmValue.tm_isdst = -1;

  time_t epoch = std::mktime(&tmValue);
  if (epoch < 0)
    return false;

  outEpoch = epoch;
  return true;
}

bool setSystemTimeFromEpoch(time_t epoch) {
  timeval tv = {};
  tv.tv_sec = epoch;
  tv.tv_usec = 0;
  return settimeofday(&tv, nullptr) == 0;
}

void applyEpochToClockState(time_t epoch) {
  std::tm tmValue = {};
  localtime_r(&epoch, &tmValue);

  app.clock.year = tmValue.tm_year + 1900;
  app.clock.month = tmValue.tm_mon + 1;
  app.clock.day = tmValue.tm_mday;
  app.clock.hour = tmValue.tm_hour;
  app.clock.minute = tmValue.tm_min;
}

void applyDraftToClockState() {
  app.clock.year = app.clock.draftYear;
  app.clock.month = app.clock.draftMonth;
  app.clock.day = app.clock.draftDay;
  app.clock.hour = app.clock.draftHour;
  app.clock.minute = app.clock.draftMinute;
  app.clock.autoSyncEnabled = app.clock.draftAutoSyncEnabled;
}

bool readSystemEpoch(time_t &outEpoch) {
  time_t epoch = std::time(nullptr);
  if (epoch <= 0)
    return false;

  outEpoch = epoch;
  return true;
}

bool refreshClockStateFromSystemTime() {
  time_t epoch = 0;
  if (!readSystemEpoch(epoch))
    return false;

  applyEpochToClockState(epoch);
  copyClockStateToDraft();
  return true;
}

bool hasUnsavedClockDraft() {
  return app.clock.autoSyncEnabled != app.clock.draftAutoSyncEnabled ||
         app.clock.year != app.clock.draftYear ||
         app.clock.month != app.clock.draftMonth ||
         app.clock.day != app.clock.draftDay ||
         app.clock.hour != app.clock.draftHour ||
         app.clock.minute != app.clock.draftMinute;
}

void persistClockRuntime(time_t epoch) {
  settingsStoreSaveClockEpoch(static_cast<unsigned long long>(epoch));
  settingsStoreSaveClockAutoSyncEnabled(app.clock.autoSyncEnabled);
  settingsStoreSaveClockSource(static_cast<uint8_t>(app.clock.source));
  settingsStoreSaveClockValid(app.clock.timeValid);
}

bool consumeNtpSyncCompleted() {
  bool completed = false;

  portENTER_CRITICAL(&ntpSyncMux);
  completed = ntpSyncCompletedPending;
  ntpSyncCompletedPending = false;
  portEXIT_CRITICAL(&ntpSyncMux);

  return completed;
}

void clearNtpSyncPending() {
  portENTER_CRITICAL(&ntpSyncMux);
  ntpSyncCompletedPending = false;
  portEXIT_CRITICAL(&ntpSyncMux);
}

bool isAutoSyncReady() {
  return app.clock.autoSyncEnabled && WiFi.status() == WL_CONNECTED;
}

bool hasReachedTime(unsigned long now, unsigned long target) {
  return static_cast<long>(now - target) >= 0;
}

void stopNtpSync() {
  ntpSyncInProgress = false;
  ntpNextAttemptMs = 0;
  clearNtpSyncPending();
  if (esp_sntp_enabled()) {
    esp_sntp_stop();
  }
}

void startNtpSync() {
  configureClockTimezone();
  if (!ntpCallbackRegistered) {
    sntp_set_time_sync_notification_cb(onSntpTimeSync);
    ntpCallbackRegistered = true;
  }

  clearNtpSyncPending();
  configTzTime(appcfg::DEFAULT_CLOCK_TZ, "pool.ntp.org", "time.nist.gov");
  ntpSyncInProgress = true;
  ntpSyncStartedMs = millis();
}
} // namespace

void clockInitializeRuntime(unsigned long long savedEpoch) {
  configureClockTimezone();

  bool initializedFromSavedEpoch = false;

  if (app.clock.timeValid && savedEpoch > 0) {
    time_t epoch = static_cast<time_t>(savedEpoch);
    if (setSystemTimeFromEpoch(epoch)) {
      applyEpochToClockState(epoch);
      initializedFromSavedEpoch = true;
    }
  }

  if (!initializedFromSavedEpoch) {
    time_t buildEpoch = 0;
    if (buildEpochFromCompileTime(buildEpoch)) {
      setSystemTimeFromEpoch(buildEpoch);
      applyEpochToClockState(buildEpoch);
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

  copyClockStateToDraft();
}

void updateClockScreen() {
  if (currentScreen != SCREEN_CLOCK)
    return;
  if (app.clock.editMode)
    return;
  if (hasUnsavedClockDraft())
    return;

  ClockStateModel previous = app.clock;
  if (!refreshClockStateFromSystemTime())
    return;

  if (previous.year != app.clock.year || previous.month != app.clock.month ||
      previous.day != app.clock.day || previous.hour != app.clock.hour ||
      previous.minute != app.clock.minute) {
    clockRender();
  }
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
    stopNtpSync();
    return;
  }

  unsigned long now = millis();

  if (!ntpSyncInProgress) {
    if (isAutoSyncReady() && hasReachedTime(now, ntpNextAttemptMs)) {
      startNtpSync();
    }
    return;
  }

  if (consumeNtpSyncCompleted()) {
    time_t epoch = 0;
    if (!readSystemEpoch(epoch)) {
      ntpSyncInProgress = false;
      ntpNextAttemptMs = now + CLOCK_NTP_RETRY_INTERVAL_MS;
      return;
    }

    if (epoch > 1700000000) {
      bool hasUnsavedDraft = hasUnsavedClockDraft();
      app.clock.source = ClockSource::Ntp;
      app.clock.timeValid = true;
      if (!hasUnsavedDraft) {
        applyEpochToClockState(epoch);
        copyClockStateToDraft();
      }
      persistClockRuntime(epoch);

      if (currentScreen == SCREEN_CLOCK && !app.clock.editMode &&
          !hasUnsavedDraft) {
        clockRender();
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

void clockRender() { drawClock(); }

void clockEnter() {
  refreshClockStateFromSystemTime();
  app.clock.selectedRow = ClockRow::Time;
  app.clock.editMode = false;
  app.clock.editPart = 0;
}

void clockHandleEncoder(bool stepPlus, bool stepMinus) {
  if (!stepPlus && !stepMinus)
    return;

  const bool plus = stepPlus;

  if (!app.clock.editMode) {
    app.clock.selectedRow = nextRow(app.clock.selectedRow, plus);
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::Time) {
    if (app.clock.editPart == 0) {
      app.clock.draftHour += plus ? 1 : -1;
      if (app.clock.draftHour < 0)
        app.clock.draftHour = 23;
      if (app.clock.draftHour > 23)
        app.clock.draftHour = 0;
    } else {
      app.clock.draftMinute += plus ? 1 : -1;
      if (app.clock.draftMinute < 0)
        app.clock.draftMinute = 59;
      if (app.clock.draftMinute > 59)
        app.clock.draftMinute = 0;
    }
    app.clock.draftAutoSyncEnabled = false;
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::Date) {
    if (app.clock.editPart == 0) {
      app.clock.draftDay += plus ? 1 : -1;
    } else if (app.clock.editPart == 1) {
      app.clock.draftMonth += plus ? 1 : -1;
    } else {
      app.clock.draftYear += plus ? 1 : -1;
    }

    clampDraftDate();
    app.clock.draftAutoSyncEnabled = false;
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::AutoSync) {
    app.clock.draftAutoSyncEnabled = !app.clock.draftAutoSyncEnabled;
    clockRender();
    return;
  }
}

void clockHandleSelect() {
  if (!app.clock.editMode) {
    app.clock.editMode = true;
    app.clock.editPart = 0;
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::Time) {
    if (app.clock.editPart == 0) {
      app.clock.editPart = 1;
    } else {
      app.clock.editMode = false;
      app.clock.editPart = 0;
    }
    clockRender();
    return;
  }

  if (app.clock.selectedRow == ClockRow::Date) {
    if (app.clock.editPart < 2) {
      app.clock.editPart++;
    } else {
      app.clock.editMode = false;
      app.clock.editPart = 0;
    }
    clockRender();
    return;
  }

  app.clock.editMode = false;
  app.clock.editPart = 0;
  clockRender();
}

void clockHandleBack() {
  bool previousAutoSyncEnabled = app.clock.autoSyncEnabled;
  bool manualTimeChanged = app.clock.year != app.clock.draftYear ||
                           app.clock.month != app.clock.draftMonth ||
                           app.clock.day != app.clock.draftDay ||
                           app.clock.hour != app.clock.draftHour ||
                           app.clock.minute != app.clock.draftMinute;

  applyDraftToClockState();

  if (manualTimeChanged) {
    app.clock.source = ClockSource::Manual;
    app.clock.autoSyncEnabled = false;
    app.clock.draftAutoSyncEnabled = false;
    stopNtpSync();
  }

  time_t epoch = 0;
  if (buildEpochFromDraft(epoch)) {
    if (manualTimeChanged) {
      app.clock.timeValid = setSystemTimeFromEpoch(epoch);
    }
    persistClockRuntime(epoch);
  } else if (manualTimeChanged) {
    app.clock.timeValid = false;
    settingsStoreSaveClockAutoSyncEnabled(app.clock.autoSyncEnabled);
    settingsStoreSaveClockSource(static_cast<uint8_t>(app.clock.source));
    settingsStoreSaveClockValid(app.clock.timeValid);
  }

  if (!manualTimeChanged && !previousAutoSyncEnabled &&
      app.clock.autoSyncEnabled && WiFi.status() == WL_CONNECTED) {
    clockRequestNtpSync();
  } else if (!app.clock.autoSyncEnabled) {
    stopNtpSync();
  }

  app.clock.editMode = false;
  app.clock.editPart = 0;
  showSettingsScreen();
}
