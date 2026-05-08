#pragma once

#include <app/app_state.h>
#include <storage/session_journal_types.h>

void drawSessionHistory(const SessionJournal &journal,
                        const SessionHistoryStateModel &state);
