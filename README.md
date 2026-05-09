# Tea Timer (ESP32-C3)

Firmware project for a tea timer on `ESP32-C3` with an OLED display (`SSD1306 128x64`), rotary encoder, dedicated back button, buzzer, and LED indicator.

Current firmware version: `1.0.0`.

## Features

- Main menu navigation with rotary encoder (`Sessions`, `Timer`, `History`,
  `Settings`).
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
  - rinse is always shown as step `0/N`; `0s` is allowed only for rinse,
  - after the first start, active session snapshot is persisted and restored
    after reboot/power loss directly to `Session Run` in `PAUSED` state,
  - while running, snapshot remaining time is refreshed at most every `5s`
    to avoid writing to flash every second,
  - back button in run: open `End session?` confirm (`No/Yes`),
  - select `Yes` in confirm: finish session early and show completion screen,
  - back button after completion: return to preset list,
  - completion screen at the end of session.
- Completed session journal:
  - completed sessions are appended to one LittleFS JSON journal,
  - false starts are not saved (`0` completed infusions),
  - journal stores up to `128` records and overwrites the oldest record first,
  - journal status is marked `pending` after each new completed session,
  - if clock time is not valid, `startedAt/finishedAt` are stored as `0`,
  - LittleFS is mounted without auto-format; mount failure makes the journal
    unavailable but does not silently erase stored history,
  - journal save is staged through three files:
    - `/session_journal.tmp` receives the newly serialized JSON first,
    - existing `/session_journal.json` is renamed to `/session_journal.bak`,
    - tmp is renamed to `/session_journal.json`,
  - if tmp write fails, the current journal is left untouched,
  - if tmp-to-journal rename fails after the current journal was moved to bak,
    the store attempts to rename bak back to `/session_journal.json`,
  - on load, the store tries `/session_journal.json`, then
    `/session_journal.bak`, then `/session_journal.tmp`; readable fallback
    files are restored to the main journal path when possible,
  - parse/version/open errors do not delete journal files automatically.
- Session History screen (`Menu -> History`):
  - shows stored completed session records,
  - select opens/closes record details,
  - encoder switches selected record,
  - long press opens delete confirm for the selected record,
  - back closes details/confirm or returns to Menu.
- Runtime status labels in header (`RUNNING`, `PAUSED`, `STOP`/`READY`) with right-aligned header text.
- Main menu header shows current time (`HH:MM`) on the right.
- Before the first successful `NTP` sync or manual time set in the current boot, menu header shows `--:--` instead of exposing stale boot-time clock data.
- Persistent settings via `Preferences` (NVS): timer preset, power save mode/timeout, audio mode/profile, clock state, active session snapshot.
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
- About screen shows chip, flash, heap, and current firmware version.

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

## Potential Improvements

- Add a battery-backed RTC module such as `DS3231` to keep accurate time across full power loss and remove the need to hide boot-time clock as `--:--` before Wi-Fi sync.
