#pragma once

void updateSessionRun();
void resetSessionFlowState();

void resetSessionLongPressFlowState();
void processSessionLongPressInput(bool down, unsigned long nowMs);

void sessionToggleRunPauseAt(unsigned long nowMs);

void sessionStopAndExitToMenu();
