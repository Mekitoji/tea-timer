#pragma once

void updateSingleTimer();
void resetSingleTimerFlowState();

int normalizeTimerPresetSec(int sec);
void applyTimerPresetSec(int sec);
void resetSingleTimerRuntimeState();

void timerLongResetToPreset();
void timerPauseAt(unsigned long nowMs);
void timerStartOrResumeAt(unsigned long nowMs);

void timerAdjustByEncoderDelta(int delta);

void resetTimerButtonFlowState();
void processTimerButtonInput(bool down, unsigned long nowMs);
