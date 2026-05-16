#include <storage/session_journal_store.h>

#include <ArduinoJson.h>
#include <LittleFS.h>

#include <cstring>

namespace {
bool mounted = false;
bool mountAttempted = false;
SessionJournal scratchJournal;

void clearRecord(SessionLogRecord &record) {
  std::memset(&record, 0, sizeof(record));
}

const char *syncStatusToString(SessionJournalSyncStatus status) {
  switch (status) {
  case SessionJournalSyncStatus::Synced:
    return "synced";
  case SessionJournalSyncStatus::Failed:
    return "failed";
  case SessionJournalSyncStatus::Pending:
  default:
    return "pending";
  }
}

SessionJournalSyncStatus syncStatusFromString(const char *value) {
  if (value && std::strcmp(value, "synced") == 0)
    return SessionJournalSyncStatus::Synced;
  if (value && std::strcmp(value, "failed") == 0)
    return SessionJournalSyncStatus::Failed;
  return SessionJournalSyncStatus::Pending;
}

bool readRecord(JsonObject recordObj, SessionLogRecord &record);
void writeRecord(JsonArray records, const SessionLogRecord &record);

bool ensureMounted() {
  if (mounted)
    return true;
  if (mountAttempted)
    return false;

  mountAttempted = true;
  mounted = LittleFS.begin(false, "/littlefs", 10, "spiffs");
  return mounted;
}

bool loadJournalFromPath(const char *path, SessionJournal &journal) {
  if (!LittleFS.exists(path))
    return false;

  File file = LittleFS.open(path, "r");
  if (!file)
    return false;

  DynamicJsonDocument doc(session_storage::JSON_DOC_CAPACITY);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error)
    return false;

  int version = doc["version"] | 0;
  if (version != session_storage::JOURNAL_VERSION)
    return false;

  JsonArray records = doc["records"].as<JsonArray>();
  if (records.isNull())
    return false;

  sessionJournalReset(journal);
  journal.version = version;
  journal.updatedAt = doc["updatedAt"] | 0UL;
  journal.syncStatus = syncStatusFromString(doc["status"] | "pending");
  journal.retryCount = doc["retryCount"] | 0;

  int index = 0;
  for (JsonObject recordObj : records) {
    if (index >= session_storage::MAX_RECORDS)
      break;

    if (readRecord(recordObj, journal.records[index]))
      index++;
  }

  journal.recordCount = index;
  return true;
}

bool restoreJournalPath(const char *fromPath) {
  if (LittleFS.exists(session_storage::JOURNAL_PATH))
    return true;
  if (!LittleFS.exists(fromPath))
    return false;
  return LittleFS.rename(fromPath, session_storage::JOURNAL_PATH);
}

bool readRecord(JsonObject recordObj, SessionLogRecord &record) {
  record = SessionLogRecord{};

  std::strncpy(record.id, recordObj["id"] | "", sizeof(record.id) - 1);
  std::strncpy(record.presetName, recordObj["presetName"] | "",
               sizeof(record.presetName) - 1);
  record.presetIndex = recordObj["presetIndex"] | 0;
  record.startedAt = recordObj["startedAt"] | 0UL;
  record.finishedAt = recordObj["finishedAt"] | 0UL;
  record.finishedEarly = recordObj["finishedEarly"] | false;
  record.completedInfusionCount = recordObj["completedInfusionCount"] | 0;
  record.rinseSec = recordObj["rinseSec"] | 0;
  record.rinseStartedAt = recordObj["rinseStartedAt"] | 0UL;

  if (record.completedInfusionCount < 0)
    record.completedInfusionCount = 0;
  if (record.completedInfusionCount > SESSION_MAX_STEPS)
    record.completedInfusionCount = SESSION_MAX_STEPS;

  JsonArray infusions = recordObj["infusionSec"].as<JsonArray>();
  int infusionIndex = 0;
  for (JsonVariant value : infusions) {
    if (infusionIndex >= record.completedInfusionCount ||
        infusionIndex >= SESSION_MAX_STEPS)
      break;
    record.infusionSec[infusionIndex++] = value.as<int>();
  }

  JsonArray infusionStarts = recordObj["infusionStartedAt"].as<JsonArray>();
  int infusionStartIndex = 0;
  for (JsonVariant value : infusionStarts) {
    if (infusionStartIndex >= record.completedInfusionCount ||
        infusionStartIndex >= SESSION_MAX_STEPS)
      break;
    record.infusionStartedAt[infusionStartIndex++] =
        value.as<unsigned long>();
  }

  return true;
}

void writeRecord(JsonArray records, const SessionLogRecord &record) {
  JsonObject recordObj = records.createNestedObject();

  recordObj["id"] = record.id;
  recordObj["presetIndex"] = record.presetIndex;
  recordObj["presetName"] = record.presetName;
  recordObj["startedAt"] = record.startedAt;
  recordObj["finishedAt"] = record.finishedAt;
  recordObj["finishedEarly"] = record.finishedEarly;
  recordObj["completedInfusionCount"] = record.completedInfusionCount;
  recordObj["rinseSec"] = record.rinseSec;
  recordObj["rinseStartedAt"] = record.rinseStartedAt;

  JsonArray infusions = recordObj.createNestedArray("infusionSec");
  for (int infusionIndex = 0;
       infusionIndex < record.completedInfusionCount &&
       infusionIndex < SESSION_MAX_STEPS;
       infusionIndex++) {
    infusions.add(record.infusionSec[infusionIndex]);
  }

  JsonArray infusionStarts = recordObj.createNestedArray("infusionStartedAt");
  for (int infusionIndex = 0;
       infusionIndex < record.completedInfusionCount &&
       infusionIndex < SESSION_MAX_STEPS;
       infusionIndex++) {
    infusionStarts.add(record.infusionStartedAt[infusionIndex]);
  }
}
} // namespace

bool sessionJournalStoreBegin() { return ensureMounted(); }

SessionJournal &sessionJournalStoreScratch() { return scratchJournal; }

bool sessionJournalExists() {
  if (!ensureMounted())
    return false;
  return LittleFS.exists(session_storage::JOURNAL_PATH) ||
         LittleFS.exists(session_storage::JOURNAL_TMP_PATH) ||
         LittleFS.exists(session_storage::JOURNAL_BACKUP_PATH);
}

bool sessionJournalDelete() {
  if (!ensureMounted())
    return false;

  bool ok = true;
  if (LittleFS.exists(session_storage::JOURNAL_TMP_PATH)) {
    ok = LittleFS.remove(session_storage::JOURNAL_TMP_PATH) && ok;
  }
  if (LittleFS.exists(session_storage::JOURNAL_BACKUP_PATH)) {
    ok = LittleFS.remove(session_storage::JOURNAL_BACKUP_PATH) && ok;
  }
  if (LittleFS.exists(session_storage::JOURNAL_PATH)) {
    ok = LittleFS.remove(session_storage::JOURNAL_PATH) && ok;
  }
  return ok;
}

void sessionJournalReset(SessionJournal &journal) {
  journal.version = session_storage::JOURNAL_VERSION;
  journal.updatedAt = 0;
  journal.syncStatus = SessionJournalSyncStatus::Pending;
  journal.retryCount = 0;
  journal.recordCount = 0;

  for (int i = 0; i < session_storage::MAX_RECORDS; i++) {
    clearRecord(journal.records[i]);
  }
}

void sessionJournalTouch(SessionJournal &journal, unsigned long updatedAt) {
  journal.updatedAt = updatedAt;
}

void sessionJournalMarkPending(SessionJournal &journal,
                               unsigned long updatedAt) {
  journal.syncStatus = SessionJournalSyncStatus::Pending;
  journal.retryCount = 0;
  journal.updatedAt = updatedAt;
}

void sessionJournalMarkSynced(SessionJournal &journal,
                              unsigned long updatedAt) {
  journal.syncStatus = SessionJournalSyncStatus::Synced;
  journal.retryCount = 0;
  journal.updatedAt = updatedAt;
}

void sessionJournalMarkFailed(SessionJournal &journal, unsigned long updatedAt) {
  journal.syncStatus = SessionJournalSyncStatus::Failed;
  journal.retryCount++;
  journal.updatedAt = updatedAt;
}

void sessionJournalAppendRecord(SessionJournal &journal,
                                const SessionLogRecord &record,
                                unsigned long updatedAt) {
  if (journal.recordCount < session_storage::MAX_RECORDS) {
    journal.records[journal.recordCount] = record;
    journal.recordCount++;
  } else {
    for (int i = 1; i < session_storage::MAX_RECORDS; i++) {
      journal.records[i - 1] = journal.records[i];
    }
    journal.records[session_storage::MAX_RECORDS - 1] = record;
  }

  sessionJournalMarkPending(journal, updatedAt);
}

bool sessionJournalDeleteRecordById(SessionJournal &journal, const char *id,
                                    unsigned long updatedAt) {
  if (!id || id[0] == '\0')
    return false;

  int foundIndex = -1;
  for (int i = 0; i < journal.recordCount; i++) {
    if (std::strcmp(journal.records[i].id, id) == 0) {
      foundIndex = i;
      break;
    }
  }

  if (foundIndex < 0)
    return false;

  for (int i = foundIndex + 1; i < journal.recordCount; i++) {
    journal.records[i - 1] = journal.records[i];
  }

  journal.recordCount--;
  if (journal.recordCount >= 0 &&
      journal.recordCount < session_storage::MAX_RECORDS) {
    clearRecord(journal.records[journal.recordCount]);
  }

  sessionJournalMarkPending(journal, updatedAt);
  return true;
}

bool sessionJournalLoad(SessionJournal &journal) {
  sessionJournalReset(journal);

  if (!ensureMounted())
    return false;

  if (loadJournalFromPath(session_storage::JOURNAL_PATH, journal))
    return true;

  if (loadJournalFromPath(session_storage::JOURNAL_BACKUP_PATH, journal)) {
    restoreJournalPath(session_storage::JOURNAL_BACKUP_PATH);
    return true;
  }

  if (loadJournalFromPath(session_storage::JOURNAL_TMP_PATH, journal)) {
    restoreJournalPath(session_storage::JOURNAL_TMP_PATH);
    return true;
  }

  sessionJournalReset(journal);
  return false;
}

bool sessionJournalSave(const SessionJournal &journal) {
  if (!ensureMounted())
    return false;

  if (journal.recordCount <= 0)
    return sessionJournalDelete();

  DynamicJsonDocument doc(session_storage::JSON_DOC_CAPACITY);
  doc["version"] = journal.version;
  doc["updatedAt"] = journal.updatedAt;
  doc["status"] = syncStatusToString(journal.syncStatus);
  doc["retryCount"] = journal.retryCount;

  JsonArray records = doc.createNestedArray("records");
  for (int i = 0; i < journal.recordCount; i++) {
    writeRecord(records, journal.records[i]);
  }

  if (doc.overflowed())
    return false;

  if (LittleFS.exists(session_storage::JOURNAL_TMP_PATH)) {
    LittleFS.remove(session_storage::JOURNAL_TMP_PATH);
  }

  File file = LittleFS.open(session_storage::JOURNAL_TMP_PATH, "w");
  if (!file)
    return false;

  if (serializeJson(doc, file) == 0) {
    file.close();
    LittleFS.remove(session_storage::JOURNAL_TMP_PATH);
    return false;
  }

  file.close();

  if (LittleFS.exists(session_storage::JOURNAL_BACKUP_PATH)) {
    LittleFS.remove(session_storage::JOURNAL_BACKUP_PATH);
  }

  bool hadCurrent = LittleFS.exists(session_storage::JOURNAL_PATH);
  if (hadCurrent &&
      !LittleFS.rename(session_storage::JOURNAL_PATH,
                       session_storage::JOURNAL_BACKUP_PATH)) {
    LittleFS.remove(session_storage::JOURNAL_TMP_PATH);
    return false;
  }

  if (!LittleFS.rename(session_storage::JOURNAL_TMP_PATH,
                       session_storage::JOURNAL_PATH)) {
    if (hadCurrent &&
        LittleFS.exists(session_storage::JOURNAL_BACKUP_PATH)) {
      LittleFS.rename(session_storage::JOURNAL_BACKUP_PATH,
                      session_storage::JOURNAL_PATH);
    }
    return false;
  }

  if (LittleFS.exists(session_storage::JOURNAL_BACKUP_PATH)) {
    LittleFS.remove(session_storage::JOURNAL_BACKUP_PATH);
  }

  return true;
}
