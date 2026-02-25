#pragma once

enum class TimerState { Stopped, Running, Paused };

extern TimerState timerState;

void setTimerStateStopped();
void setTimerStateRunning();
void setTimerStatePaused();

bool isTimerStopped();
bool isTimerRunning();
bool isTimerPaused();
