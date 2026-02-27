#pragma once

void loadSessionPresetByIndex(int presetIndex);
void enterSessionRunFromCurrentPreset();

void updateSessionRun();
void resetSessionFlowState();

void resetSessionLongPressFlowState();
void processSessionLongPressInput(bool down, unsigned long nowMs);

void sessionToggleRunPauseAt(unsigned long nowMs);
