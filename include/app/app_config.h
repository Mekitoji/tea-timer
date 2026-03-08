#pragma once

namespace appcfg {
inline constexpr char PREFS_NAMESPACE[] = "tea_timer";
inline constexpr char PREFS_DURATION_KEY[] = "dur_sec";
inline constexpr char PREFS_POWER_SAVE_KEY[] = "pwr_save";

inline constexpr int DEFAULT_TIMER_DURATION = 10;
inline constexpr bool DEFAULT_POWER_SAVE_ENABLED = true;

inline constexpr unsigned long INPUT_DEBOUNCE_MS = 30;

inline constexpr unsigned long TIMER_HOLD_MS = 1200;

inline constexpr unsigned long ENC_ACCEL_FAST_MS = 80;
inline constexpr unsigned long ENC_ACCEL_MEDIUM_MS = 140;
inline constexpr int ENC_STEP_NORMAL = 1;
inline constexpr int ENC_STEP_MEDIUM = 5;
inline constexpr int ENC_STEP_FAST = 10;

inline constexpr unsigned long SESSION_HOLD_MS = 1200;
inline constexpr unsigned long SESSION_RELEASE_SETTLE_MS = 10;
inline constexpr unsigned long WIFI_HOLD_MS = 1200;

inline constexpr unsigned long DISPLAY_IDLE_OFF_MS = 60000;
inline constexpr bool ENABLE_LIGHT_SLEEP = true;
inline constexpr unsigned long LIGHT_SLEEP_IDLE_MS = 120000;

inline constexpr unsigned long WIFI_RECONNECT_INTERVAL_MS = 8000;
} // namespace appcfg
