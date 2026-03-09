#pragma once

void settingsStoreBegin();

int settingsStoreLoadTimerDurationSec();
void settingsStoreSaveTimerDurationSec(int seconds);

bool settingsStoreLoadPowerSaveEnabled();
void settingsStoreSavePowerSaveEnabled(bool enabled);
