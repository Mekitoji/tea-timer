#pragma once

#include <app/app_config.h>

void settingsStoreBegin();

int settingsStoreLoadTimerDurationSec();
void settingsStoreSaveTimerDurationSec(int seconds);

bool settingsStoreLoadPowerSaveEnabled();
void settingsStoreSavePowerSaveEnabled(bool enabled);

BeepProfile settingsStoreLoadBeepProfile();
void settingsStoreSaveBeepProfile(BeepProfile profile);

bool settingsStoreLoadAudioEnabled();
void settingsStoreSaveAudioEnabled(bool enabled);

unsigned long settingsStoreLoadDisplayIdleOffMs();
void settingsStoreSaveDisplayIdleOffMs(unsigned long ms);

bool settingsStoreLoadClockAutoSyncEnabled();
void settingsStoreSaveClockAutoSyncEnabled(bool enabled);

uint8_t settingsStoreLoadClockSource();
void settingsStoreSaveClockSource(uint8_t source);

bool settingsStoreLoadClockValid();
void settingsStoreSaveClockValid(bool valid);

unsigned long long settingsStoreLoadClockEpoch();
void settingsStoreSaveClockEpoch(unsigned long long epoch);
