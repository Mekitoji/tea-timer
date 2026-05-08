#pragma once

#include <storage/session_journal_types.h>

bool sessionJournalStoreBegin();
bool sessionJournalLoad(SessionJournal &journal);
bool sessionJournalSave(const SessionJournal &journal);
bool sessionJournalDelete();
bool sessionJournalExists();

void sessionJournalReset(SessionJournal &journal);
void sessionJournalTouch(SessionJournal &journal, unsigned long updatedAt);
void sessionJournalMarkPending(SessionJournal &journal, unsigned long updatedAt);
void sessionJournalMarkSynced(SessionJournal &journal, unsigned long updatedAt);
void sessionJournalMarkFailed(SessionJournal &journal, unsigned long updatedAt);
void sessionJournalAppendRecord(SessionJournal &journal,
                                const SessionLogRecord &record,
                                unsigned long updatedAt);
bool sessionJournalDeleteRecordById(SessionJournal &journal, const char *id,
                                    unsigned long updatedAt);
SessionJournal &sessionJournalStoreScratch();
