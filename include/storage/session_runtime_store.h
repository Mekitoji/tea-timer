#pragma once

#include <app/session_runtime_snapshot.h>

bool sessionRuntimeStoreLoad(SessionRuntimeSnapshot &snapshot);
void sessionRuntimeStoreSave(const SessionRuntimeSnapshot &snapshot);
void sessionRuntimeStoreClear();
