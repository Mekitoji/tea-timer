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
