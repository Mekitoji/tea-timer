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

### Pinout (from `include/pins.h`)

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
