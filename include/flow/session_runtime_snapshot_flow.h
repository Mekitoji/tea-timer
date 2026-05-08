#pragma once

bool restoreSessionRuntimeSnapshotOnBoot();
void persistSessionRuntimeSnapshot(unsigned long nowMs);
void clearSessionRuntimeSnapshot();
