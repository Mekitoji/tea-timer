#pragma once

#include <app/app_state.h>
#include <app/session_runtime_snapshot.h>

SessionRuntimeSnapshot buildSessionRuntimeSnapshot(
    const SessionStateModel &session, SessionState state,
    unsigned long remainingSec);

bool applySessionRuntimeSnapshot(const SessionRuntimeSnapshot &snapshot,
                                 SessionStateModel &session);
