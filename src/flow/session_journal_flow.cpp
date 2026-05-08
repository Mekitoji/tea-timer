#include <flow/session_journal_flow.h>

#include <Arduino.h>
#include <UUID7.h>
#include <app/app_state.h>
#include <app/clock_time.h>
#include <app/session_log.h>
#include <app/session_presets.h>
#include <storage/session_journal_store.h>

#include <cstring>

namespace {
bool generateRecordId(char *out, size_t outSize) {
  if (!out || outSize < sizeof(SessionLogRecord::id))
    return false;

  UUID7 uuid;
  uuid.setVersion(UUID_VERSION_4);

  if (!uuid.generate())
    return false;

  return uuid.toString(out, outSize);
}

int completedInfusionCountFromSession(bool includeCurrentInfusion) {
  if (app.session.rinseActive)
    return 0;

  int count = app.session.stepIndex;
  if (includeCurrentInfusion)
    count++;

  if (count < 0)
    count = 0;
  if (count > app.session.stepCount)
    count = app.session.stepCount;
  if (count > SESSION_MAX_STEPS)
    count = SESSION_MAX_STEPS;

  return count;
}

bool buildSessionLogRecord(bool finishedEarly, SessionLogRecord &record) {
  record = SessionLogRecord{};

  if (!app.session.started)
    return false;

  int completedInfusions = completedInfusionCountFromSession(finishedEarly);
  if (completedInfusions <= 0)
    return false;

  if (!generateRecordId(record.id, sizeof(record.id)))
    return false;

  record.presetIndex = app.session.presetIndex;

  if (SESSION_PRESET_COUNT > 0 && app.session.presetIndex >= 0 &&
      app.session.presetIndex < SESSION_PRESET_COUNT) {
    std::strncpy(record.presetName, SESSION_PRESETS[app.session.presetIndex].name,
                 sizeof(record.presetName) - 1);
  }

  record.startedAt = app.session.startedAt;
  record.finishedAt = clockCurrentEpochOrZero();
  record.finishedEarly = finishedEarly;
  record.completedInfusionCount = completedInfusions;
  record.rinseSec = app.session.rinseSec;

  for (int i = 0; i < completedInfusions; i++) {
    record.infusionSec[i] = app.session.steps[i];
  }

  return true;
}
} // namespace

bool persistCompletedSessionJournalRecord(bool finishedEarly) {
  SessionLogRecord record;
  if (!buildSessionLogRecord(finishedEarly, record))
    return false;

  SessionJournal &journal = sessionJournalStoreScratch();
  if (!sessionJournalLoad(journal))
    sessionJournalReset(journal);

  unsigned long updatedAt = clockCurrentEpochOrZero();
  sessionJournalAppendRecord(journal, record, updatedAt);
  return sessionJournalSave(journal);
}
