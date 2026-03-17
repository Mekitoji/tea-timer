#pragma once

#include <ctime>

#include <app/app_state.h>

void clockConfigureTimezone();
int clockDaysInMonth(int year, int month);
void clockClampDraftDate(ClockStateModel &clockState);
void clockCopyStateToDraft(ClockStateModel &clockState);
void clockApplyDraftToState(ClockStateModel &clockState);
bool clockHasUnsavedDraft(const ClockStateModel &clockState);
bool clockBuildEpochFromCompileTime(time_t &outEpoch);
bool clockBuildEpochFromDraft(const ClockStateModel &clockState, time_t &outEpoch);
bool clockSetSystemTimeFromEpoch(time_t epoch);
void clockApplyEpochToState(ClockStateModel &clockState, time_t epoch);
