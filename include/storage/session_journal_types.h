#pragma once

#include <cstdint>

#include <app/session_log.h>
#include <storage/session_storage_config.h>

enum class SessionJournalSyncStatus : uint8_t {
  Pending = 0,
  Synced = 1,
  Failed = 2
};

struct SessionJournal {
  int version = session_storage::JOURNAL_VERSION;
  unsigned long updatedAt = 0;
  SessionJournalSyncStatus syncStatus = SessionJournalSyncStatus::Pending;
  uint8_t retryCount = 0;

  int recordCount = 0;
  SessionLogRecord records[session_storage::MAX_RECORDS] = {};
};
