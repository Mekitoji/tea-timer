#pragma once

namespace appcfg {
inline constexpr char PREFS_NAMESPACE[] = "tea_timer";
inline constexpr char PREFS_DURATION_KEY[] = "dur_sec";
inline constexpr int DEFAULT_TIMER_DURATION = 10;

inline constexpr unsigned long INPUT_DEBOUNCE_MS = 30;

inline constexpr unsigned long TIMER_HOLD_MS = 1200;

inline constexpr unsigned long ENC_ACCEL_FAST_MS = 80;
inline constexpr unsigned long ENC_ACCEL_MEDIUM_MS = 140;
inline constexpr int ENC_STEP_NORMAL = 1;
inline constexpr int ENC_STEP_MEDIUM = 5;
inline constexpr int ENC_STEP_FAST = 10;

inline constexpr unsigned long SESSION_HOLD_MS = 1200;
inline constexpr unsigned long SESSION_RELEASE_SETTLE_MS = 10;
} // namespace appcfg
