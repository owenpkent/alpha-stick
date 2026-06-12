# LLM Onboarding

Quick reference for AI assistants working on Alpha Stick.

---

## Project Overview

**Name:** Alpha Stick  
**Purpose:** Open-source adaptive gaming joystick using ESP32 and 3D printing  
**Status:** Planning / Early Development

---

## About the Owner

I'm Owen — a wheelchair user with muscular dystrophy.

- **Typing is hard** — Be proactive. Make decisions. Don't ask for confirmation on small things.
- **Offer A/B/C choices** — I can type one letter instead of explaining.
- **PowerShell on Windows** — Use PowerShell syntax. Prefer single-line commands.
- **Accessibility matters** — This project is a tool I actually need.

---

## Project Goals

1. **Affordable** — Target BOM under $50
2. **Open Source** — Hardware, firmware, and docs freely available
3. **Customizable** — Modular, adaptable to different disabilities
4. **Compatible** — Works with Xbox Adaptive Controller, PC, and Project Nimbus
5. **Updateable** — OTA firmware updates

---

## Key Files

| File | Purpose |
|------|---------|
| `README.md` | Project overview, features, quick start |
| `TODO.md` | Task tracking with checkboxes |
| `docs/DESIGN_V2.md` | V2 design baseline: rationale, budgets, roadmap |
| `docs/HARDWARE.md` | Build reference: parts, pins, wiring, BOM |
| `docs/FIRMWARE.md` | Firmware architecture (ESP-IDF), build instructions |
| `docs/PRINTING.md` | 3D printing guide and settings |
| `firmware/sdkconfig.defaults` | ESP-IDF build configuration |
| `firmware/main/main.cpp` | Firmware entry point |
| `models/stl/` | Ready-to-print 3D models |

---

## Tech Stack

- **Microcontroller:** ESP32-S3 (USB HID + Bluetooth LE)
- **Framework:** ESP-IDF 5.x (TinyUSB + NimBLE); moved off Arduino/PlatformIO in V2
- **Protocols:** USB composite HID, BLE HID, ESP-NOW, UART AS-Link (ATOS), WiFi (for config)
- **3D Modeling:** FreeCAD or Fusion360
- **PCB Design:** KiCad or EasyEDA

**V2 design baseline:** `docs/DESIGN_V2.md` is the source of truth (contactless dual-Hall
sensing, magnetic centering 1-8 gf, <5 gf force target, sensor pod architecture). HARDWARE.md
and FIRMWARE.md match it. ISOMETRIC_PARTS.md is archived.

---

## Related Project: Project Nimbus

Alpha Stick integrates with [Project Nimbus](https://github.com/owenpkent/Project-Nimbus):

- **Location:** `C:\Users\Owen\dev\Project-Nimbus`
- **Purpose:** Virtual controller interface using vJoy
- **Integration:** Alpha Stick provides physical input, Nimbus adds on-screen controls
- **Shared concepts:** Sensitivity curves, deadzone, profile system

---

## Conventions

### Commit Messages
```
feat: add new feature
fix: correct bug
docs: update documentation
refactor: restructure code
chore: maintenance tasks
```

### Code Style
- ESP32 firmware: C++ on ESP-IDF 5.x (FreeRTOS tasks, components)
- Web config: HTML/CSS/JS (vanilla, no frameworks)
- Python scripts: PEP 8

### File Naming
- Lowercase with hyphens: `alpha-stick`, `web-config`
- STL files: `enclosure-base-v1.stl`
- Source files: `main.cpp`, `joystick.h`

---

## Hardware Components

| Component | Recommended Part | Purpose |
|-----------|-----------------|---------|
| MCU | ESP32-S3-MINI-1 (DevKitC for bench) | Main controller |
| Position sensing | 2x TMAG5273 3D Hall + N52 diametric magnet | Contactless, <5 gf stick |
| Centering | Ring magnet on threaded carrier | Adjustable 1-8 gf return force |
| Buttons | 6mm tactile | Direct input |
| Jacks | PJ-320A 3.5mm | External switches |
| Case | 3D printed PLA/PETG | Enclosure + sensor pod bodies |

---

## Quick Commands

```powershell
# Build firmware (ESP-IDF PowerShell)
cd C:\Users\Owen\dev\alpha-stick\firmware; idf.py build

# Flash and monitor
idf.py -p COM5 flash monitor

# Open in VS Code
code C:\Users\Owen\dev\alpha-stick
```

---

## Inspiration Products

| Product | Website | Key Features |
|---------|---------|--------------|
| QuadStick | quadstick.com | Mouth-operated, sip/puff |
| Feather | celticmagic.org/feather | Ultra-light force, magnetic |
| Xbox Adaptive | xbox.com | 3.5mm jacks, USB hub |
| PS Access | playstation.com | Swappable caps, profiles |
| Titan Two | consoletuner.com | Cross-platform, scripting |

---

## Current Focus

Phase 1: Foundation
1. Define hardware requirements
2. Select components
3. Create initial 3D model
4. Basic firmware scaffold

See `TODO.md` for detailed task tracking.

---

## Constellation

This repo is tracked by [Constellation](https://github.com/owenpkent/constellation).

For dashboard integration:
1. `README.md` with `## Status` section ✅
2. `TODO.md` with checkbox items ✅
3. Add to Constellation's `projects.yaml`
]]>
