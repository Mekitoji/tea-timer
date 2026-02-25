#include <app/app_state.h>
#include <app/session_state.h>

SessionState sessionState = SessionState::Stopped;

void setSessionStateStopped() { sessionState = SessionState::Stopped; }

void setSessionStateRunning() { sessionState = SessionState::Running; }

void setSessionStatePaused() { sessionState = SessionState::Paused; }

void setSessionStateCompleted() { sessionState = SessionState::Completed; }

bool isSessionStopped() { return sessionState == SessionState::Stopped; }
bool isSessionRunning() { return sessionState == SessionState::Running; }
bool isSessionPaused() { return sessionState == SessionState::Paused; }
bool isSessionCompleted() { return sessionState == SessionState::Completed; }
