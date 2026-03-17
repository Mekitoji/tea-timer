#pragma once

#include <ctime>

void clockInitializeRuntime(unsigned long long savedEpoch);
void clockPersistState(time_t epoch);
void clockRefreshStateFromSystemTime();
void clockRequestNtpSync();
void clockCancelNtpSync();
void updateClockRuntime();
void updateClockScreen();
