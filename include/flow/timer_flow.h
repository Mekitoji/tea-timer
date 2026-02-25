#pragma once

void updateSingleTimer();
void resetSingleTimerFlowState();

int normalizeTimerPresetSec(int sec);
void applyTimerPresetSec(int sec);
void resetSingleTimerRuntimeState();
