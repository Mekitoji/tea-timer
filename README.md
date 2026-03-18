# Tea Timer (ESP32-C3)

Firmware project for a tea timer on `ESP32-C3` with an OLED display (`SSD1306 128x64`), rotary encoder, dedicated back button, buzzer, and LED indicator.

## Features

- Main menu navigation with rotary encoder (`Sessions`, `Timer`, `Settings`).
- Single Timer mode:
  - adjust seconds with encoder acceleration (`+1 / +5 / +10` by rotation speed),
  - short press on encoder: start/pause/resume,
  - long press on encoder: reset to current preset,
  - back button: return to menu.
- Session mode:
  - presets-based flow (`Menu -> Sessions -> Preset -> Session Run`),
  - 15 tea presets with dose/temperature/infusions metadata,
  - short press: start/pause/resume current step,
  - encoder in paused state: adjust current step duration,
  - long press: skip current step,
  - rinse is a separate optional step (if `rinse=0`, rinse is not shown),
  - back button in run: open `End session?` confirm (`No/Yes`),
  - select `Yes` in confirm: finish session early and show completion screen,
  - back button after completion: return to preset list,
  - completion screen at the end of session.
- Runtime status labels in header (`RUNNING`, `PAUSED`, `STOP`/`READY`) with right-aligned header text.
- Main menu header shows current time (`HH:MM`) on the right.
- Before the first successful `NTP` sync or manual time set in the current boot, menu header shows `--:--` instead of exposing stale boot-time clock data.
- Persistent settings via `Preferences` (NVS): timer preset, power save mode/timeout, audio mode/profile, clock state.
- Power Save mode (Settings):
  - toggle `ON/OFF`,
  - configurable display idle timeout (`15s / 30s / 60s / 120s / 300s`),
  - idle OLED off,
  - optional ESP32 light sleep on longer idle,
  - first wake input is consumed.
- Audio mode (Settings):
  - toggle `ON/OFF`,
  - beep profile selection (`Soft / Normal / Loud`).
- Clock mode (Settings):
  - manual `Time` and `Date` editing,
  - `Auto Sync` toggle,
  - manual edit immediately disables `Auto Sync`,
  - when Wi-Fi is connected and `Auto Sync = ON`, device syncs time from NTP,
  - after successful sync, periodic resync is scheduled automatically.
  - on boot without fresh sync, device restores last known time for internal clock state, but UI hides it as `--:--` until current-boot manual set or NTP sync.
  - sync flow:
    - Wi-Fi connected -> request NTP sync,
    - clock runtime polls SNTP sync status in `loop()`,
    - after `SNTP_SYNC_STATUS_COMPLETED`, runtime reads system time and stores it in `Preferences`,
    - next background resync is scheduled in `6h`,
    - if sync does not complete within `15s`, current attempt is stopped and retry is scheduled in `30s`,
    - manual `Time/Date` change disables `Auto Sync` and cancels pending NTP sync.
- Phone Wi-Fi provisioning (BLE-first via `WiFiProv`):
  - single screen `Settings -> WiFi` (no separate status screen),
  - if credentials are missing: BLE setup mode is shown (`PROV_xxxxxx`, POP),
  - if credentials exist: current STA status/SSID/IP/RSSI is shown,
  - long press on Wi-Fi screen opens `Reset Wi-Fi?` confirm (`No/Yes`),
  - selecting `Yes` clears saved STA config and switches to BLE setup.
- Non-blocking input handling with debounce and encoder quadrature decoding.
- Explicit FSM layers:
  - `TimerState`: `Stopped/Running/Paused`,
  - `SessionState`: `Stopped/Running/Paused/Completed`.

## Requirements

### Hardware

- `ESP32-C3-DevKitM-1` board.
- `SSD1306` OLED over I2C (address `0x3C`).
- Rotary encoder with push button.
- Dedicated `Back` button.
- Piezo buzzer.
- LED (preferably with a current-limiting resistor).
- USB cable for flashing.

### Pinout (from `include/hw/pins.h`)

- OLED: `SDA=GPIO8`, `SCL=GPIO9`
- Encoder: `A=GPIO2`, `B=GPIO3`, `SW=GPIO1`
- Back button: `GPIO4`
- Buzzer: `GPIO10`
- LED: `GPIO7`

### Software

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) (`pio` command) or the PlatformIO extension for VS Code.
- USB driver for your board (if required by your OS).

## How to Run

1. Build the project:

```bash
pio run
```

1. Flash the board:

```bash
pio run -t upload
```

1. Open Serial Monitor (`115200`):

```bash
pio device monitor -b 115200
```

If needed, specify the port manually:

```bash
pio run -t upload --upload-port <PORT>
pio device monitor -b 115200 --port <PORT>
```

## Wi-Fi Setup (Phone, BLE)

`SoftAP provisioning for this hardware setup was unstable in practice (AP visibility/connection drops), so BLE provisioning is the primary and supported flow.`

1. On device open `Settings -> Wi-Fi`.
2. Open Espressif provisioning app on phone (`ESP BLE Provisioning` / `ESP SoftAP Provisioning`).
3. Choose BLE transport and select device name shown on timer screen (`PROV_xxxxxx`).
4. Enter POP `teatimer`.
5. Enter your home `SSID` and `Password`, then submit.
6. Wait for device status `CONNECTED` and verify shown STA IP.

Notes:

- If Wi-Fi credentials already exist, the same screen shows connection status instead of setup prompt.
- `Back` exits Wi-Fi screen.
- Saved credentials are reused on next boot (best-effort reconnect).
- To reprovision, long-press on Wi-Fi screen, confirm reset, then run BLE setup again.

## Project Structure

- `src/main.cpp` — app bootstrap (`setup/loop`) and top-level orchestration.
- `src/app/app_controller.cpp` — central input dispatcher by screen/controller.
- `src/app/app_state.cpp` — global app state storage and menu item tables.
- `src/app/session_presets.cpp` — tea session presets (metadata + infusion steps).
- `src/app/timer_state.cpp` — timer FSM implementation (`Stopped/Running/Paused`).
- `src/app/session_state.cpp` — session FSM implementation (`Stopped/Running/Paused/Completed`).
- `src/controllers/menu_controller.cpp` — menu input handling.
- `src/controllers/timer_controller.cpp` — timer screen input handling.
- `src/controllers/session_controller.cpp` — session screens input handling.
- `src/controllers/settings_controller.cpp` — settings subtree input handling.
- `src/controllers/wifi_controller.cpp` — Wi-Fi screen input handling.
- `src/flow/navigation_flow.cpp` — centralized screen transitions (`navigateTo`, back behavior).
- `src/flow/timer_flow.cpp` — single timer runtime/update logic.
- `src/flow/session_flow.cpp` — session runtime/update logic.
- `src/flow/wifi_flow.cpp` — BLE provisioning + STA status/reconnect logic.
- `src/flow/clock_flow.cpp` — Clock screen editor flow (rows, edit mode, save/cancel).
- `src/flow/clock_runtime.cpp` — Clock boot/runtime logic, NTP sync scheduling, and live screen refresh.
- `src/flow/power_flow.cpp` — idle display off/light sleep/wake guard runtime logic.
- `src/flow/power_settings_flow.cpp` — Power Save settings editor flow.
- `src/flow/audio_profile_flow.cpp` — profile-based audio frequencies/durations.
- `src/flow/audio_settings_flow.cpp` — Audio settings editor flow.
- `src/storage/settings_store.cpp` — persistent settings load/save wrappers over `Preferences`.
- `src/app/clock_time.cpp` — clock date/time helpers (`epoch <-> state`, timezone, draft/date clamp).
- `src/hw/input.cpp` — encoder/buttons (debounced, non-blocking) + acceleration helper.
- `src/hw/audio.cpp` — buzzer on/off helpers.
- `src/hw/feedback.cpp` — LED + audio pulse helper.
- `src/ui/menu.cpp` — main/settings menu drawing.
- `src/ui/header.cpp` — shared header rendering.
- `src/ui/timer.cpp` — timer screen rendering.
- `src/ui/session.cpp` — session preset/run/complete rendering.
- `src/ui/settings/wifi.cpp` — Wi-Fi settings rendering.
- `src/ui/settings/clock.cpp` — Clock settings rendering.
- `src/ui/settings/power_save.cpp` — Power Save settings rendering.
- `src/ui/settings/audio.cpp` — Audio settings rendering.
- `src/ui/settings/about.cpp` — About screen rendering.
- `include/app/app_config.h` — app-level constants (timings, prefs keys, defaults).
- `include/app/app_state.h` — app models/state declarations.
- `include/app/session_presets.h` — session preset declarations.
- `include/app/timer_state.h` — timer FSM declarations.
- `include/app/session_state.h` — session FSM declarations.
- `include/app/app_controller.h` — top-level input handling API.
- `include/controllers/*.h` — per-screen input controller APIs.
- `include/flow/navigation_flow.h` — navigation API.
- `include/flow/timer_flow.h` — timer flow API.
- `include/flow/session_flow.h` — session flow API.
- `include/flow/wifi_flow.h` — Wi-Fi flow API.
- `include/flow/clock_flow.h` — Clock screen editor flow API.
- `include/flow/clock_runtime.h` — Clock runtime/NTP/update API.
- `include/flow/power_flow.h` — power runtime API.
- `include/flow/power_settings_flow.h` — Power Save settings flow API.
- `include/flow/audio_profile_flow.h` — audio profile mapping API.
- `include/flow/audio_settings_flow.h` — Audio settings flow API.
- `include/storage/settings_store.h` — settings persistence API.
- `include/app/clock_time.h` — clock date/time helper API.
- `include/hw/pins.h` — hardware pin mapping.
- `include/hw/input.h` — input API.
- `include/hw/audio.h` — buzzer API.
- `include/hw/feedback.h` — LED/audio feedback API.

## Potential Improvements

- Add a battery-backed RTC module such as `DS3231` to keep accurate time across full power loss and remove the need to hide boot-time clock as `--:--` before Wi-Fi sync.
