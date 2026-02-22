# Tea Timer (ESP32-C3)

Firmware project for a tea timer on `ESP32-C3` with an OLED display (`SSD1306 128x64`), rotary encoder, dedicated back button, buzzer, and LED indicator.

## Features

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

## Project Structure

- `src/main.cpp` — app bootstrap (`setup/loop`) and module orchestration.
- `src/app/app_state.cpp` — shared app state definitions.
- `src/app/app_controller.cpp` — input-driven screen/control handlers.
- `src/hw/input.cpp` — encoder/buttons (debounced, non-blocking).
- `src/hw/audio.cpp` — buzzer helpers.
- `src/flow/menu_flow.cpp` — main menu actions and screen transitions.
- `src/flow/timer_flow.cpp` — single timer runtime logic.
- `src/flow/session_flow.cpp` — session runtime logic.
- `src/ui/*.cpp` — UI rendering split by domain:
  - `ui/menu.cpp`
  - `ui/timer.cpp`
  - `ui/info.cpp`
  - `ui/session.cpp`
- `include/app/app_config.h` — app-level constants (prefs keys, debounce/hold timings, encoder accel).
- `include/app/app_state.h` — shared app state declarations.
