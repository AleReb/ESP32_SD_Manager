# ESP32 SD Manager

A lightweight web-based SD card file manager for ESP32. This small library exposes a Wi‑Fi access point and an HTTP server that allows you to:

- List files on an SD card, showing name and size
- Download files with progress indication
- Upload new files with progress bar
- Rename or delete existing files via web UI

## Features

1. **Standalone AP & server**: No internet connection required. ESP32 creates an AP named `ESP32 SD Manager`.
2. **Modern Web UI**: HTML/CSS/JavaScript with progress bars and action buttons.
3. **Single-header integration**: Copy the `.ino` sketch into your project and call `setupSDWebManager()` from `setup()`.

## Hardware

- **ESP32** (any board with HSPI pins available)
- **MicroSD card module** wired to HSPI:
  - **SD_MISO** ➔ GPIO 2
  - **SD_MOSI** ➔ GPIO 15
  - **SD_SCLK** ➔ GPIO 14
  - **SD_CS**   ➔ GPIO 13

## Installation

1. Clone or download this repository.
2. Copy `SdWebFileList.ino` into your Arduino sketch folder.
3. Include required libraries in Arduino IDE:
   ```cpp
   #include <WiFi.h>
   #include <WebServer.h>
   #include <SD.h>
   #include <SPI.h>
   #include <FS.h>
   ```

## Usage

1. **Customize** (optional): change `ssid` and `password` constants in the sketch.
2. **Upload** to ESP32 via Arduino IDE.
3. **Power on** and wait for serial output:
   ```
   SD initialized
   AP IP: 192.168.4.1
   HTTP server started
   ```
4. **Connect** your computer or smartphone to Wi‑Fi network `ESP32 SD Manager` (default password `12345678`).
5. **Browse** to `http://192.168.4.1/` to manage SD card files.

## Integration as Function

Extract the web‑manager logic into a function:
```cpp
void setupSDWebManager() {
  // SD & SPI init, Wi‑Fi AP, server routes, server.begin()
}
```
Call in `setup()`:
```cpp
void setup() {
  Serial.begin(115200);
  setupSDWebManager();
}
```

## License

MIT © 2025
