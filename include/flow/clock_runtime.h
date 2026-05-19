#pragma once

#include <ctime>

enum class ClockSyncUiState {
  Off,
  Synced,
  Syncing,
  Waiting,
  WaitingWifi,
  WaitingRetry,
  Failed,
};

void clockInitializeRuntime(unsigned long long savedEpoch);
void clockPersistState(time_t epoch);
void clockRefreshStateFromSystemTime();
void clockRequestNtpSync();
void clockCancelNtpSync();
ClockSyncUiState clockSyncUiState();
void updateClockRuntime();
void updateClockScreen();
