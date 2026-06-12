# Alpha Stick Firmware

ESP-IDF 5.x scaffold implementing the architecture in [docs/FIRMWARE.md](../docs/FIRMWARE.md).

> **Status: compiles clean in CI (ESP-IDF v5.3.2, esp32s3); not yet run on hardware.**
> The structure, task graph, pipeline math, and AS-Link wire format are real. The TMAG5273
> register values are written from datasheet memory and flagged with VERIFY comments;
> compiling proves nothing about them. The remaining Phase 0 firmware spikes
> ([TODO.md](../TODO.md)) are on-device: flash, enumerate, verify the simulation demo,
> then measure.

## Simulation mode

By default (`AS_SIMULATE_WHEN_NO_SENSOR=y`), if no TMAG5273 answers on I2C the sensing task
synthesizes a slow circle with a periodic Z-press. That means a **bare ESP32-S3 devkit with no
wiring at all** should enumerate as a gamepad+mouse+keyboard composite device and draw circles
in `joy.cpl`. This is the Phase 0 TinyUSB spike with zero soldering.

## Build

Use an ESP-IDF 5.x PowerShell (Espressif Windows installer):

```powershell
cd C:\Users\Owen\dev\alpha-stick\firmware; idf.py set-target esp32s3
idf.py build
idf.py -p COM5 flash monitor
```

DevKitC-1 has two USB connectors: flash/monitor via the **UART** connector; TinyUSB owns the
**USB (native)** connector. Plug both in during development.

Options live under `menuconfig -> Alpha Stick`:

| Option | Values | Notes |
|--------|--------|-------|
| Build profile | GAMING / DRIVE | DRIVE tightens safety behavior (see docs/DESIGN_V2.md sec. 9); scaffold only logs the difference so far |
| Role | STICK / DONGLE | DONGLE (AS-Link to USB bridge) is a stub |
| Simulate when no sensor | y/n | The bare-devkit demo switch |

## Layout

| Path | Contents | State |
|------|----------|-------|
| `main/` | boot, task bring-up | working skeleton |
| `components/as_sensing/` | TMAG5273 driver, sensing task, seqlock mailbox, simulation | driver needs bench verify |
| `components/as_pipeline/` | profile types, radial deadzone, expo curve, 1-euro filter | implemented, host-testable math |
| `components/as_modes/` | 1 kHz router task, gamepad/mouse mapping, Z-click hysteresis | gamepad + mouse working paths |
| `components/as_hid_usb/` | TinyUSB composite descriptors (16-bit gamepad + mouse + keyboard + CDC) | needs first enumeration test |
| `components/as_hid_ble/` | BLE HID | stub |
| `components/as_aslink/` | wire format header (vendorable into ATOS), CRC32, COBS | implemented, host-testable |
| `components/as_config/` | profile defaults, NVS persistence | minimal working |
| `components/as_web/` | WiFi AP + web UI | stub |

## Bring-up checklist

1. `idf.py build` (CI already proves this passes on ESP-IDF v5.3.2).
2. Flash a bare devkit, open Windows `joy.cpl`: gamepad circles from simulation mode.
3. Wire a TMAG5273 breakout (SDA 8, SCL 9, 3V3, GND), wave a magnet: log line flips from
   SIMULATION to LIVE and the axes follow the magnet.
4. Verify the TMAG5273 register writes against the datasheet (marked VERIFY in
   `tmag5273.cpp`) before trusting any noise numbers.
