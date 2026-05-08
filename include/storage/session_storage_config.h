#pragma once

#include <cstddef>

namespace session_storage {
inline constexpr int JOURNAL_VERSION = 1;
inline constexpr int MAX_RECORDS = 128;
inline constexpr size_t JSON_DOC_CAPACITY = 65536;

inline constexpr char JOURNAL_PATH[] = "/session_journal.json";
inline constexpr char JOURNAL_TMP_PATH[] = "/session_journal.tmp";
inline constexpr char JOURNAL_BACKUP_PATH[] = "/session_journal.bak";
} // namespace session_storage
