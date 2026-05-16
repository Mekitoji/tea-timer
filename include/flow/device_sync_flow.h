#pragma once

#include <cstdint>
#include <storage/session_journal_types.h>

enum class DeviceSyncFlowState : uint8_t {
  Idle = 0,
  Syncing = 1,
  Synced = 2,
  Failed = 3
};

struct DeviceSyncSnapshot {
  DeviceSyncFlowState state = DeviceSyncFlowState::Idle;
  SessionJournalSyncStatus journalStatus = SessionJournalSyncStatus::Pending;
  int recordCount = 0;
  int acceptedRecordCount = 0;
  char message[48] = {};
};

void deviceSyncFlowInit();
void requestDeviceSyncNow();
void updateDeviceSyncFlow();
void deviceSyncSnapshot(DeviceSyncSnapshot &snapshot);
