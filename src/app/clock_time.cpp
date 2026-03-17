#include <app/clock_time.h>

#include <cstring>
#include <sys/time.h>

#include <app/app_config.h>

void clockConfigureTimezone() {
  setenv("TZ", appcfg::DEFAULT_CLOCK_TZ, 1);
  tzset();
}

int clockDaysInMonth(int year, int month) {
  if (month == 2) {
    bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    return leap ? 29 : 28;
  }
  if (month == 4 || month == 6 || month == 9 || month == 11)
    return 30;
  return 31;
}

void clockClampDraftDate(ClockStateModel &clockState) {
  if (clockState.draftMonth < 1)
    clockState.draftMonth = 12;
  if (clockState.draftMonth > 12)
    clockState.draftMonth = 1;

  if (clockState.draftYear < 2024)
    clockState.draftYear = 2024;
  if (clockState.draftYear > 2099)
    clockState.draftYear = 2099;

  int maxDay = clockDaysInMonth(clockState.draftYear, clockState.draftMonth);
  if (clockState.draftDay < 1)
    clockState.draftDay = maxDay;
  if (clockState.draftDay > maxDay)
    clockState.draftDay = 1;
}

void clockCopyStateToDraft(ClockStateModel &clockState) {
  clockState.draftAutoSyncEnabled = clockState.autoSyncEnabled;
  clockState.draftYear = clockState.year;
  clockState.draftMonth = clockState.month;
  clockState.draftDay = clockState.day;
  clockState.draftHour = clockState.hour;
  clockState.draftMinute = clockState.minute;
}

void clockApplyDraftToState(ClockStateModel &clockState) {
  clockState.year = clockState.draftYear;
  clockState.month = clockState.draftMonth;
  clockState.day = clockState.draftDay;
  clockState.hour = clockState.draftHour;
  clockState.minute = clockState.draftMinute;
  clockState.autoSyncEnabled = clockState.draftAutoSyncEnabled;
}

bool clockHasUnsavedDraft(const ClockStateModel &clockState) {
  return clockState.autoSyncEnabled != clockState.draftAutoSyncEnabled ||
         clockState.year != clockState.draftYear ||
         clockState.month != clockState.draftMonth ||
         clockState.day != clockState.draftDay ||
         clockState.hour != clockState.draftHour ||
         clockState.minute != clockState.draftMinute;
}

namespace {
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
} // namespace

bool clockBuildEpochFromCompileTime(time_t &outEpoch) {
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

bool clockBuildEpochFromDraft(const ClockStateModel &clockState,
                              time_t &outEpoch) {
  std::tm tmValue = {};
  tmValue.tm_year = clockState.draftYear - 1900;
  tmValue.tm_mon = clockState.draftMonth - 1;
  tmValue.tm_mday = clockState.draftDay;
  tmValue.tm_hour = clockState.draftHour;
  tmValue.tm_min = clockState.draftMinute;
  tmValue.tm_sec = 0;
  tmValue.tm_isdst = -1;

  time_t epoch = std::mktime(&tmValue);
  if (epoch < 0)
    return false;

  outEpoch = epoch;
  return true;
}

bool clockSetSystemTimeFromEpoch(time_t epoch) {
  timeval tv = {};
  tv.tv_sec = epoch;
  tv.tv_usec = 0;
  return settimeofday(&tv, nullptr) == 0;
}

void clockApplyEpochToState(ClockStateModel &clockState, time_t epoch) {
  std::tm tmValue = {};
  localtime_r(&epoch, &tmValue);

  clockState.year = tmValue.tm_year + 1900;
  clockState.month = tmValue.tm_mon + 1;
  clockState.day = tmValue.tm_mday;
  clockState.hour = tmValue.tm_hour;
  clockState.minute = tmValue.tm_min;
}
