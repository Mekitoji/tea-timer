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
- Persistent timer preset via `Preferences` (NVS).
- Power Save mode (Settings):
  - idle OLED off,
  - optional ESP32 light sleep on longer idle,
  - first wake input is consumed
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

2. Flash the board:

```bash
pio run -t upload
```

3. Open Serial Monitor (`115200`):

```bash
pio device monitor -b 115200
```

If needed, specify the port manually:

```bash
pio run -t upload --upload-port <PORT>
pio device monitor -b 115200 --port <PORT>
```

## Project Structure

- `src/main.cpp` — app bootstrap (`setup/loop`) and module orchestration.
- `src/app/app_state.cpp` — shared app state definitions.
- `src/app/session_presets.cpp` — session preset definitions (steps + metadata).
- `src/app/timer_state.cpp` — `TimerState` implementation.
- `src/app/session_state.cpp` — `SessionState` implementation.
- `src/app/app_controller.cpp` — input-driven screen/control handlers.
- `src/hw/input.cpp` — encoder/buttons (debounced, non-blocking) + encoder acceleration helper.
- `src/hw/audio.cpp` — buzzer helpers.
- `src/flow/menu_flow.cpp` — main menu actions and screen transitions.
- `src/flow/timer_flow.cpp` — single timer runtime logic.
- `src/flow/session_flow.cpp` — session runtime logic.
- `src/flow/power_flow.cpp` — idle display off/light sleep and wake-guard logic.
- `src/ui/*.cpp` — UI rendering split by domain:
  - `ui/menu.cpp`
  - `ui/header.cpp`
  - `ui/timer.cpp`
  - `ui/info.cpp`
  - `ui/session.cpp`
- `include/app/app_config.h` — app-level constants (prefs keys, debounce/hold timings, encoder accel).
- `include/app/app_state.h` — shared app state declarations.
- `include/app/session_presets.h` — session preset model/declarations.
- `include/app/timer_state.h` — `TimerState` declarations.
- `include/app/session_state.h` — `SessionState` declarations.
- `include/flow/timer_flow.h` — timer flow API.
- `include/flow/session_flow.h` — session flow API.
- `include/flow/power_flow.h` — power-saving flow API.
