#pragma once

enum class SessionState { Stopped, Running, Paused, Completed };

extern SessionState sessionState;

void setSessionStateStopped();
void setSessionStateRunning();
void setSessionStatePaused();
void setSessionStateCompleted();

bool isSessionStopped();
bool isSessionRunning();
bool isSessionPaused();
bool isSessionCompleted();
