#pragma once

void markUserActivity();
bool markUserActivityAndConsumeIfWoke();
bool isWakeInputGuardActive();

void updatePowerSaving();
void setPowerSavingEnabled(bool enabled);
bool isPowerSavingEnabled();
void initPowerSaving();

void setDisplayIdleOffTimeoutMs(unsigned long timeoutMs);
unsigned long getDisplayIdleOffTimeoutMs();
