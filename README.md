# ESP32 SD Manager V0.2

**Lightweight web-based SD card file manager for ESP32**

ESP32 SD Manager creates a standalone Wi-Fi Access Point and HTTP server
to manage files on a microSD card from any browser—no external network needed.

## 🚀 Features

1. **Standalone AP & HTTP Server**  
   - ESP32 in AP mode (default SSID `ESP32_SD_ManagerV0.2`)  
   - No internet connection required  

2. **Modern Web UI**  
   - HTML/CSS/JavaScript  
   - Progress bars for uploads/downloads  
   - Action buttons for folder navigation, file ops  

3. **Complete File Operations**  
   - List files/directories (with size)  
   - Download files with progress indication  
   - View text (txt, csv, log, json, xml, html, js, css, md)  
   - View images (jpg, png, gif, bmp)  
   - Upload new files (drag & drop style)  
   - Create folders, rename, delete (recursive for directories)

4. **Safety & Info Notices**  
   - Warnings before deleting entire folders  
   - Info panel for supported file types

5. **Single-Header Integration**  
   - Copy the `.ino` sketch into any project  
   - Optionally extract into `setupSDWebManager()` for reuse

## 📋 Hardware

- **ESP32** (any board with HSPI pins)
- **MicroSD card module** wired to HSPI:

  | Signal | ESP32 Pin |
  |--------|-----------|
  | MISO   | GPIO19    |
  | MOSI   | GPIO23    |
  | SCLK   | GPIO18    |
  | CS     | GPIO5     |

> **Tip:** Use the secondary HSPI SPIClass (`spiSD`) for SD to avoid conflicts.

## ⚙️ Installation

1. **Clone** or download this repository.  
2. **Copy** `SDManager.ino` into your Arduino sketch folder.  
3. **Include** required libraries:

   ```cpp
   #include <WiFi.h>
   #include <WebServer.h>
   #include <SD.h>
   #include <SPI.h>
   #include <FS.h>
