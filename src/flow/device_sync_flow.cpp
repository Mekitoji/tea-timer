#include <flow/device_sync_flow.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <app/app_config.h>
#include <app/app_state.h>
#include <app/clock_time.h>
#include <app/session_state.h>
#include <app/timer_state.h>
#include <net/api_client.h>
#include <storage/device_auth_store.h>
#include <storage/session_journal_store.h>

#include <cstring>

namespace {
DeviceSyncFlowState syncState = DeviceSyncFlowState::Idle;
char lastMessage[48] = {};
int lastAcceptedRecordCount = 0;
unsigned long nextAttemptMs = 0;
bool forceSync = false;

void copyCString(char *out, size_t outSize, const char *value) {
  if (!out || outSize == 0)
    return;

  std::strncpy(out, value ? value : "", outSize - 1);
  out[outSize - 1] = '\0';
}

void setMessage(const char *message) {
  copyCString(lastMessage, sizeof(lastMessage), message);
}

const char *journalStatusString(SessionJournalSyncStatus status) {
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

String errorMessageFromResponse(const ApiResponse &response) {
  if (!response.body.isEmpty()) {
    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, response.body) == DeserializationError::Ok) {
      const char *message = doc["error"]["message"] | nullptr;
      if (message && message[0] != '\0')
        return String(message);
    }
  }

  if (!response.error.isEmpty())
    return response.error;

  String message("HTTP ");
  message += response.statusCode;
  return message;
}

unsigned long retryDelayMs(const SessionJournal &journal) {
  unsigned long multiplier = journal.retryCount;
  if (multiplier < 1)
    multiplier = 1;
  if (multiplier > 10)
    multiplier = 10;

  unsigned long delay = appcfg::CLOUD_SYNC_RETRY_MS * multiplier;
  if (delay > appcfg::CLOUD_SYNC_MAX_RETRY_MS)
    delay = appcfg::CLOUD_SYNC_MAX_RETRY_MS;
  return delay;
}

bool buildSyncBody(const SessionJournal &journal, String &outBody) {
  DynamicJsonDocument doc(session_storage::JSON_DOC_CAPACITY);
  doc["schemaVersion"] = 1;

  JsonObject journalObj = doc.createNestedObject("journal");
  journalObj["version"] = journal.version;
  journalObj["updatedAt"] = journal.updatedAt;
  journalObj["status"] = journalStatusString(journal.syncStatus);
  journalObj["retryCount"] = journal.retryCount;

  JsonArray records = journalObj.createNestedArray("records");
  for (int i = 0; i < journal.recordCount; i++) {
    const SessionLogRecord &record = journal.records[i];
    JsonObject recordObj = records.createNestedObject();

    recordObj["id"] = record.id;
    recordObj["presetIndex"] = record.presetIndex;
    recordObj["presetName"] =
        record.presetName[0] ? record.presetName : "Session";
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

  if (doc.overflowed()) {
    setMessage("Sync JSON too large");
    return false;
  }

  outBody = "";
  serializeJson(doc, outBody);
  return true;
}

void markJournalFailed(SessionJournal &journal, const char *message) {
  sessionJournalMarkFailed(journal, clockCurrentEpochOrZero());
  sessionJournalSave(journal);
  syncState = DeviceSyncFlowState::Failed;
  setMessage(message);
  nextAttemptMs = millis() + retryDelayMs(journal);
}

void performSync(SessionJournal &journal,
                 const DeviceAuthCredentials &credentials) {
  String body;
  if (!buildSyncBody(journal, body)) {
    markJournalFailed(journal, lastMessage);
    return;
  }

  syncState = DeviceSyncFlowState::Syncing;
  setMessage("Syncing");

  ApiResponse response = apiClientPostJson("/device-sync/session-journal", body,
                                           credentials.deviceToken);

  if (!response.ok()) {
    String message = errorMessageFromResponse(response);
    markJournalFailed(journal, message.c_str());
    return;
  }

  DynamicJsonDocument responseDoc(512);
  DeserializationError error = deserializeJson(responseDoc, response.body);
  if (error) {
    markJournalFailed(journal, "Bad sync response");
    return;
  }

  bool ok = responseDoc["ok"] | false;
  const char *status = responseDoc["status"] | "";
  if (!ok || std::strcmp(status, "synced") != 0) {
    markJournalFailed(journal, "Sync rejected");
    return;
  }

  lastAcceptedRecordCount = responseDoc["acceptedRecordCount"] | 0;
  unsigned long serverTime = responseDoc["serverTime"] | 0UL;
  sessionJournalMarkSynced(
      journal, serverTime > 0 ? serverTime : clockCurrentEpochOrZero());
  sessionJournalSave(journal);

  syncState = DeviceSyncFlowState::Synced;
  setMessage("Synced");
  nextAttemptMs = 0;
}

bool shouldSyncJournal(const SessionJournal &journal) {
  if (journal.recordCount <= 0)
    return false;

  return journal.syncStatus == SessionJournalSyncStatus::Pending ||
         journal.syncStatus == SessionJournalSyncStatus::Failed;
}

void scheduleIdleScan() {
  nextAttemptMs = millis() + appcfg::CLOUD_SYNC_IDLE_SCAN_MS;
}
} // namespace

void deviceSyncFlowInit() {
  syncState = DeviceSyncFlowState::Idle;
  lastAcceptedRecordCount = 0;
  nextAttemptMs = 0;
  forceSync = false;
  setMessage("Idle");
}

void requestDeviceSyncNow() { forceSync = true; }

void updateDeviceSyncFlow() {
  if (!forceSync && nextAttemptMs > 0 && millis() < nextAttemptMs)
    return;
  if (isTimerRunning() || isSessionRunning())
    return;
  if (WiFi.status() != WL_CONNECTED)
    return;
  if (currentScreen == SCREEN_SESSION_HISTORY)
    return;

  DeviceAuthCredentials credentials;
  if (!deviceAuthStoreLoad(credentials)) {
    syncState = DeviceSyncFlowState::Idle;
    setMessage("Not paired");
    scheduleIdleScan();
    forceSync = false;
    return;
  }

  SessionJournal &journal = sessionJournalStoreScratch();
  if (!sessionJournalLoad(journal)) {
    syncState = DeviceSyncFlowState::Idle;
    setMessage("No journal");
    scheduleIdleScan();
    forceSync = false;
    return;
  }

  if (!shouldSyncJournal(journal)) {
    syncState = journal.syncStatus == SessionJournalSyncStatus::Synced
                    ? DeviceSyncFlowState::Synced
                    : DeviceSyncFlowState::Idle;
    setMessage(journal.recordCount > 0 ? "Synced" : "No records");
    scheduleIdleScan();
    forceSync = false;
    return;
  }

  forceSync = false;
  performSync(journal, credentials);
}

void deviceSyncSnapshot(DeviceSyncSnapshot &snapshot) {
  snapshot = DeviceSyncSnapshot{};
  snapshot.state = syncState;
  snapshot.acceptedRecordCount = lastAcceptedRecordCount;
  copyCString(snapshot.message, sizeof(snapshot.message), lastMessage);

  SessionJournal &journal = sessionJournalStoreScratch();
  if (sessionJournalLoad(journal)) {
    snapshot.journalStatus = journal.syncStatus;
    snapshot.recordCount = journal.recordCount;
  }
}
