#pragma once

void markUserActivity();
bool isWakeInputGuardActive();
void updatePowerSaving();
bool markUserActivityAndConsumeIfWoke();
void setPowerSavingEnabled(bool enabled);
bool isPowerSavingEnabled();
void initPowerSaving();
