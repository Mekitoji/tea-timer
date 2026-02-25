#include <app/timer_state.h>

TimerState timerState = TimerState::Stopped;

void setTimerStateStopped() { timerState = TimerState::Stopped; }

void setTimerStateRunning() { timerState = TimerState::Running; }

void setTimerStatePaused() { timerState = TimerState::Paused; }

bool isTimerStopped() { return timerState == TimerState::Stopped; }
bool isTimerRunning() { return timerState == TimerState::Running; }
bool isTimerPaused() { return timerState == TimerState::Paused; }
