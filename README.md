
# 🖥️ Smart-Display-Canvas — Wireless Interactive 7-Segment LED Matrix

> A wireless, browser-controlled digital canvas built on an **ESP32** and a daisy-chain of **8 × MAX7219** LED drivers, turning 64 individual 7-segment displays into a real-time drawable art board.

![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![Language](https://img.shields.io/badge/firmware-C%2B%2B%20(Arduino)-00979D)
![Driver](https://img.shields.io/badge/driver-MAX7219-red)
![Connectivity](https://img.shields.io/badge/connectivity-WiFi%20AP%20Mode-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 📖 Table of Contents

1. [Abstract](#-abstract)
2. [Key Features](#-key-features)
3. [System Architecture](#-system-architecture)
4. [Hardware Components](#-hardware-components)
5. [Pin Diagrams & Wiring](#-pin-diagrams--wiring)
6. [Power Design & Calculations](#-power-design--calculations)
7. [Software Stack](#-software-stack)
8. [Getting Started](#-getting-started)
9. [Firmware Overview](#-firmware-overview)
10. [Web Interface Details](#-web-interface-details)
11. [Results](#-results)
12. [Project Team Workflow](#-project-team-workflow)
13. [Known Limitations & Future Work](#-known-limitations--future-work)
14. [Abbreviations](#-abbreviations)
15. [References](#-references)
16. [License](#-license)

---

## 🧾 Abstract

This project implements a **wireless interactive display system** using an **ESP32** microcontroller and multiple **MAX7219**-driven 7-segment display modules. The system hosts a **web-based drawing canvas** directly from the ESP32 operating in **Access Point (AP) mode** — no external router or internet connection required. Drawing input captured in the browser is streamed in real time to the ESP32, which decodes the coordinates and activates the corresponding LED segments across a daisy-chained MAX7219 array.

The result is a retro-style, IoT-enabled visualization board that demonstrates practical integration of **embedded hardware**, **wireless communication**, and **web technologies** for real-time human–machine interaction.

---
<img width="720" height="720" alt="66412113-a378-4ff5-ae25-fc93fb04ff55" src="https://github.com/user-attachments/assets/4cdbb87c-d9c2-40aa-8582-6e5bbf877c9f" />
<img width="720" height="720" alt="43ac929c-ce6d-450b-b532-0a175d991d34" src="https://github.com/user-attachments/assets/e38838e6-58ad-41c6-bb96-9f706690b2ba" />
<img width="720" height="720" alt="60614750-40b5-4536-b4a6-e5f8ae500776" src="https://github.com/user-attachments/assets/3824bfd3-4b13-43a6-a481-65bc33e3ca43" />




<img width="720" height="720" alt="a459635f-0bb3-48a3-bc7d-a23bd89c56f5" src="https://github.com/user-attachments/assets/570c01fd-e71d-4947-b7e1-abbc78af6878" />
<img width="720" height="720" alt="bacf8af3-225c-45fa-be3e-9164b41045fc" src="https://github.com/user-attachments/assets/536e91e7-37f6-4a15-bae7-f5d8df0613ef" />

https://github.com/user-attachments/assets/1e50633f-7fd4-4bf5-923b-ab28a4358492

https://github.com/user-attachments/assets/5e35a55f-52e8-4050-b6ef-8c6fed39ed79
## ✨ Key Features

| Feature | Description |
|---|---|
| 🎨 **Live Web Canvas** | Sketch directly from a phone or laptop browser — no app install needed |
| 📡 **Standalone Wi-Fi AP** | ESP32 broadcasts its own network (`Smart-Display-Canvas` / `Vertical-Canvas`); works with zero internet infrastructure |
| 🔗 **Daisy-Chained Drivers** | 8 × MAX7219 modules controlled via a single SPI-style bus, minimizing GPIO usage |
| ⚡ **Real-Time Rendering** | Pointer events are throttled and mapped to individual LED segments with near-instant physical feedback |
| 🧹 **One-Tap Clear** | Dedicated `/clear` endpoint wipes both the display buffer and the physical matrix |
| 🔋 **Isolated Power Domain** | Display array runs off a dedicated 5V rail, decoupled from the ESP32 to prevent brown-outs |
| 🧩 **Segment-Level Addressing** | Every one of the 512 individual LEDs (64 displays × 8 segments) is independently toggleable |

---

## 🏗️ System Architecture

```
┌─────────────────────┐        Wi-Fi (AP Mode)        ┌──────────────────────┐
│  Remote User Device  │ ─────────────────────────────▶ │        ESP32          │
│ (Smartphone / Laptop)│ ◀───────────────────────────── │  (Web Server + Logic) │
└─────────────────────┘        HTTP GET /draw           └──────────┬───────────┘
                                                                     │ SPI (DIN/CLK/CS)
                                                                     ▼
                                                        ┌────────────────────────┐
                                                        │  MAX7219 Module 1       │
                                                        │  (DOUT) ──▶ Module 2    │
                                                        │  (DOUT) ──▶ Module 3    │
                                                        │        ...             │
                                                        │  (DOUT) ──▶ Module 8    │
                                                        └────────────────────────┘
                                                                     │
                                                                     ▼
                                                        8 × 8 Matrix of 7-Segment
                                                        Displays (64 total, 512 LEDs)
```

**Data flow summary:**
1. User draws on the HTML canvas served by the ESP32.
2. Pointer coordinates are converted client-side into `module`, `digit`, and `segment` indices.
3. An HTTP `GET /draw?m=&d=&s=` request is fired to the ESP32.
4. The ESP32 updates an in-memory `displayState[MAX_DEVICES][8]` buffer and pushes it to the MAX7219 chain via the `MD_MAX72xx` library.
5. The physical LED matrix updates instantly to mirror the browser canvas.

---

## 🔩 Hardware Components

| Component | Quantity | Purpose |
|---|---|---|
| ESP32 Development Board | 1 | Central MCU — Wi-Fi AP, web server, SPI master |
| MAX7219 LED Display Driver Module | 8 | Drives 8 × 7-segment displays each (64 total) |
| 9V DC Power Supply | 1 | Primary input power source |
| 5V DC Buck Converter | 1 | Steps down 9V → 5V for the display array |
| Jumper Wires | — | Daisy-chain and control-line connections |
| Remote User Device | 1+ | Any Wi-Fi-enabled smartphone or laptop for drawing |

---

## 🔌 Pin Diagrams & Wiring

### MAX7219 Pinout

| Pin | Function | Pin | Function |
|---|---|---|---|
| 1 | DIN | 24 | DOUT |
| 2 | DIG 0 | 23 | SEG D |
| 3 | DIG 4 | 22 | SEG DP |
| 4 | GND | 21 | SEG E |
| 5 | DIG 6 | 20 | SEG C |
| 6 | DIG 2 | 19 | V+ |
| 7 | DIG 3 | 18 | ISET |
| 8 | DIG 7 | 17 | SEG G |
| 9 | GND | 16 | SEG B |
| 10 | DIG 5 | 15 | SEG F |
| 11 | DIG 1 | 14 | SEG A |
| 12 | LOAD (CS) | 13 | CLK |

### ESP32 → MAX7219 Chain Wiring

| ESP32 Pin | MAX7219 Signal | Notes |
|---|---|---|
| GPIO23 | DIN (MOSI) | Data into **Module 1** only |
| GPIO18 | CLK (SCK) | Shared clock across all 8 modules |
| GPIO5 | CS (Chip Select / LOAD) | Shared across all 8 modules |
| GND | GND | Common ground — **must** tie ESP32 GND, buck-converter GND, and all module GNDs together |

> ⚠️ **Critical wiring rule:** Only **Module 1**'s `DIN` connects to the ESP32. Every subsequent module's `DIN` connects to the **previous module's `DOUT`** (daisy-chain). All 8 modules share the same `CLK`, `CS`, and 5V/GND rails.

**Firmware pin mapping (final code):**
```cpp
#define CLK_PIN   18
#define DATA_PIN  23
#define CS_PIN    5
#define MAX_DEVICES 4   // set to 8 for the full 8x8 matrix
```

---

## 🔋 Power Design & Calculations

The display array draws significantly more current than the ESP32 can safely source, so it is powered independently.

| Metric | Value | Basis |
|---|---|---|
| Per-segment draw | ~20 mA | Datasheet typical LED forward current |
| LEDs per display | 8 | 7 segments + 1 decimal point |
| Total LEDs (8×8 matrix) | **512** | 64 displays × 8 LEDs |
| Avg. segments lit / display | 3 (assumed dynamic load) | Typical canvas usage pattern |
| **Estimated dynamic current** | **3.84 A** | `3 × 20 mA × 64 displays` |
| Max current per MAX7219 | 160 mA | `8 segments × 20 mA` |
| Max current, 8-driver chain | ~1.28 A | `8 × 160 mA` |
| ESP32 Wi-Fi active draw | ~100 mA | Active AP + web server operation |

**Conclusion:** Since dynamic loads regularly exceed **1 A**, a discrete **5V buck converter fed by a 9V supply** is required — the display array must **never** be powered from the ESP32's onboard regulator, which would brown-out the MCU or damage the board.

---

## 🖥️ Software Stack

| Layer | Technology |
|---|---|
| Firmware | C++ (Arduino framework for ESP32) |
| LED Driver Library | [`MD_MAX72xx`](https://github.com/MajicDesigns/MD_MAX72XX) |
| Networking | `WiFi.h` (SoftAP mode) + `WebServer.h` |
| Frontend | Vanilla HTML / CSS / JavaScript served inline from ESP32 flash |
| Input Handling | Pointer Events API (`onpointerdown` / `onpointermove`), throttled to ~35 ms |
| IDE / Toolchain | Arduino IDE with ESP32 board support |

---

## 🚀 Getting Started

### 1. Prerequisites
- Arduino IDE (with **ESP32 board package** installed)
- Libraries: `MD_MAX72xx`, `WiFi` (bundled), `WebServer` (bundled), `SPI` (bundled)
- ESP32 dev board + USB cable
- 8 × MAX7219 modules, 9V supply, 5V buck converter

### 2. Wiring
Follow the [Pin Diagrams & Wiring](#-pin-diagrams--wiring) section above. Double-check common ground between the ESP32, buck converter, and all MAX7219 modules before powering on.

### 3. Flashing the Firmware
```bash
# In Arduino IDE:
# 1. Select Board: "ESP32 Dev Module"
# 2. Select the correct COM/serial port
# 3. Install MD_MAX72xx via Library Manager
# 4. Open the .ino sketch (see /Code Implementation below)
# 5. Click Upload
```

### 4. Connecting & Drawing
1. Power on the board — it starts broadcasting a Wi-Fi network (`Smart-Display-Canvas` or `Vertical-Canvas`, password `12345678`).
2. On your phone/laptop, connect to that network.
3. Open a browser and navigate to the ESP32's AP IP (printed on Serial Monitor at boot — typically `192.168.4.1`).
4. Draw on the on-screen canvas — the physical LED matrix updates in real time.
5. Tap **CLEAR ALL** / **ERASE ALL** to reset the display.

---

## 🧠 Firmware Overview

### Core Data Structure
```cpp
byte displayState[MAX_DEVICES][8];
```
Each module maintains an 8-byte column buffer (one byte per digit position), where each bit represents one LED segment's on/off state.

### Key Functions

| Function | Role |
|---|---|
| `updateHardware()` | Pushes `displayState[][]` to the physical MAX7219 chain via `mx.setColumn()` |
| `handleDraw()` | HTTP handler for `/draw?m=&d=&s=` — validates bounds, toggles/sets the target bit, calls `updateHardware()` |
| `handleClear` (lambda in `/clear`) | Clears both `mx` hardware buffer and the `displayState` array via `memset` |
| `setup()` | Initializes SPI/MAX7219 (`mx.begin()`), disables shutdown mode, sets LED intensity, starts the SoftAP and HTTP routes |
| `loop()` | Continuously services incoming HTTP requests with `server.handleClient()` |

### Two Firmware Variants Included

1. **Multi-Module Toggle Canvas** (`Smart-Display-Canvas`) — each segment tap **toggles** (`^=`) its state; segments are addressed via a discrete on-screen 8×8 grid of digit/segment "buttons."
2. **Vertical Freehand Canvas** (`Vertical-Canvas`) — a continuous whiteboard-style surface where pointer movement is geometrically mapped to the nearest segment and **sets** (`|=`) it active, simulating freehand drawing across the matrix.

> 💡 The segment-geometry mapping in the freehand variant (`sBit` calculation) approximates which of the 7 segments a given `(x, y)` touch point falls nearest to within a simulated 7-segment "digit" cell, using proportional zone thresholds (`ly`, `dy`) rather than fixed pixel boxes.

---

## 🌐 Web Interface Details

| Route | Method | Purpose |
|---|---|---|
| `/` | GET | Serves the embedded HTML/CSS/JS drawing canvas |
| `/draw` | GET | Accepts `m` (module), `d` (digit), `s` (segment) query params; updates one LED segment |
| `/clear` | GET | Clears the entire matrix and resets the state buffer |

**Frontend highlights:**
- Fully self-contained — HTML/CSS/JS is stored as a `R"rawliteral(...)"` C++ string and served directly from flash memory (no SPIFFS/LittleFS required).
- Responsive `viewport` meta tags for mobile-first drawing.
- Retro terminal aesthetic (`#0f0` green-on-black) matching the physical LED matrix's visual character.
- Pointer-event throttling (35 ms) prevents flooding the ESP32's web server with excessive requests during freehand drawing.

---

## 📊 Results

- ✅ Successfully built a functional **8×8 interactive 7-segment display matrix** (64 displays, 512 individually addressable LEDs).
- ✅ Demonstrated end-to-end **wireless data pipeline**: browser input → Wi-Fi transmission → ESP32 processing → serial MAX7219 output → physical LED activation.
- ✅ Validated **power isolation design**, confirming stable operation under dynamic loads without ESP32 brown-outs.
- ✅ Rendered legible **alphanumeric characters** (e.g., "T", "E", "C", "S") on the physical matrix during final-phase testing, confirming correct segment-to-pixel mapping.

*(See the original project documentation for the corresponding photos of the 4×8 integration stage and final-phase character rendering.)*

---

## 👥 Project Team Workflow

The project was divided across **three members** into parallel tracks:

| Track | Responsibility |
|---|---|
| **Embedded Hardware** | Wiring and assembling the physical circuit (ESP32, MAX7219 chain, power distribution) |
| **Firmware** | Writing the C++ logic for the ESP32 (SPI driver control, HTTP handling, state management) |
| **IoT / Web** | Developing the web interface and the wireless data link between browser and MCU |

These tracks were integrated into a single functional device as the project's final deliverable.

---

## 🔮 Known Limitations & Future Work

- Current dynamic-load estimate (3 segments/display average) is a design approximation, not a measured worst-case value — a full-brightness all-segments-on scenario would approach the driver's theoretical ceiling and should be tested with a bench multimeter.
- No persistent storage — drawings are lost on power cycle or `/clear`; a future revision could add SPIFFS/LittleFS-based save/load of canvas states.
- AP-mode only — no bridging to an existing home/lab Wi-Fi network (STA mode) for remote access beyond the local hotspot range.
- Segment-mapping in the freehand variant is a geometric approximation; touch-precision could be improved with calibration.
- No authentication on the `/draw` and `/clear` endpoints — anyone connected to the AP can control the display.

---

## 🔤 Abbreviations

| Abbreviation | Full Form |
|---|---|
| IoT | Internet of Things |
| LED | Light Emitting Diode |
| MCU | Micro-Controller Unit |
| AP mode | Access Point mode |
| DC | Direct Current |
| GPIO | General Purpose Input/Output |

---

## 📚 References

1. ESP32 Datasheet — [Google Drive link](https://drive.google.com/file/d/1YYEfnvFbxy2HQlfT4294Hevo23jeGdGm/view?usp=drive_link)
2. MAX7219 Datasheet — [Google Drive link](https://drive.google.com/file/d/19JXIDpoE1UKZpfFHsclFvC_H47Maiv8i/view?usp=drive_link)
3. [`MD_MAX72xx` Arduino Library](https://github.com/MajicDesigns/MD_MAX72XX) — MAX7219/7221 LED matrix driver

---

---

<p align="center"><i>Built as part of an ECE embedded systems + IoT integration project — combining hardware, firmware, and web technologies into one interactive device.</i></p>
