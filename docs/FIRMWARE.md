# Firmware Documentation

Firmware reference for Alpha Stick **V2**, targeting **ESP-IDF 5.x** on ESP32-S3.
Design rationale lives in [DESIGN_V2.md](DESIGN_V2.md) (especially sections 7-9).

V1 of this document described an Arduino/PlatformIO architecture. V2 moves to ESP-IDF because
TinyUSB composite devices, NimBLE multi-report HID, ESP-NOW, and OTA-with-rollback are all
first class there, and because ATOS uses the same toolchain on the same chip, so components and
build knowledge transfer directly. Arduino remains fine for throwaway sensor bring-up spikes.

---

## What the firmware does

One composite input device, four personalities, switched at runtime without re-enumerating:

| Mode | Output |
|------|--------|
| Gamepad | 16-bit X/Y stick, Z-press + jacks + sip/puff as buttons |
| Mouse | Velocity pointer with its own curve, dwell click, scroll and precision toggles |
| Keyboard | 8-way WASD/arrows with hysteresis |
| ATOS input | AS-Link frames over UART/ESP-NOW, HID silent |
| Dual | Gamepad + mouse simultaneously |

Transports: USB (TinyUSB composite: HID gamepad + mouse + keyboard report IDs, plus CDC for
config), BLE HID (same report IDs in one HID service, 3 bonds with quick switch), ESP-NOW
(to a dongle or directly to an ATOS node), UART (AS-Link wired).

---

## Project structure (ESP-IDF)

```
firmware/
  CMakeLists.txt
  sdkconfig.defaults            # S3 target, TinyUSB, NimBLE, partitions
  partitions.csv                # factory + ota_0/ota_1 + nvs + littlefs
  main/
    CMakeLists.txt
    main.cpp                    # boot, task bring-up, mode supervisor
    Kconfig.projbuild           # AS_BUILD_PROFILE: GAMING / DRIVE, role: STICK / DONGLE
  components/
    as_sensing/                 # TMAG5273 driver, dual-read, cross-check, calibration map
    as_pipeline/                # tare, rotation, deadzone, curves, 1-euro, notch, slew
    as_modes/                   # gamepad / mouse / keyboard / atos routers
    as_hid_usb/                 # TinyUSB descriptors + report writers
    as_hid_ble/                 # NimBLE HID service
    as_aslink/                  # frame definition, COBS, CRC32, UART + ESP-NOW transports
    as_config/                  # profiles (JSON in NVS/LittleFS), schema, migration
    as_web/                     # WiFi AP, web UI, OTA endpoints (config mode only)
  web/                          # web UI source, built into the LittleFS image
  tools/                        # calibration sweep helper, latency rig scripts
```

`as_aslink/include/aslink_frame.h` is the single source of truth for the wire format and is
written to be vendored into the ATOS gateway module unchanged.

---

## Task architecture

```
 sensor_task (core 1, prio 23, 1 kHz)
   INT-driven reads of both TMAG5273 -> cross-check -> calibration map
   -> pipeline -> seqlock mailbox (never blocks, never allocates)

 router_task (core 0, prio 20, on new sample)
   mailbox -> active mode mapping -> usb_hid / ble_hid / aslink writers

 tinyusb task (core 0, prio 21)        nimble host task (core 0, prio 18)
 aslink_tx   (core 0, prio 21, 200 Hz-1 kHz, drive: fixed rate + health 1 Hz)
 ui_task     (core 0, prio 10, 50 Hz)  LEDs, haptics, button combos
 config/web  (core 0, prio 5)          only in config mode (WiFi AP up)

 task watchdog: fed by sensor_task only when the 1 kHz deadline is met
 and sensor data is fresh; a starved producer reboots rather than streams stale input
```

Producer/consumer handoff is a latest-value **seqlock mailbox** (same pattern as ATOS's clamped
setpoint mailbox): the 1 kHz producer never waits on USB, BLE, or UART.

### Core data type

```cpp
// as_sensing/include/stick_sample.h
struct StickSample {
    float    x, y;        // normalized [-1, +1], post-pipeline
    float    z;           // press axis [0, +1]
    uint32_t t_us;        // sample timestamp
    bool     cal_valid;   // calibration map loaded and in range
    bool     dual_ok;     // sensors agree within threshold
    bool     tare_stable; // center reference trustworthy
};
```

---

## Sensing

- Both TMAG5273 run continuous 3-axis conversions with 8x averaging (~3 kSPS raw), INT pins
  signal conversion-ready; reads are non-blocking on I2C0 at 1 MHz.
- Tilt is computed from **field direction** (normalized vector through the calibration map),
  making it first-order immune to temperature and gap creep (DESIGN_V2 section 4).
- **Cross-check:** per-sample disagreement between sensors, low-pass tracked. Gaming build:
  log + average. Drive build: >3% FS for >50 ms latches a fault (output zero, recover only at
  neutral + explicit ack).
- **Z-press:** |B| gap detection with hysteresis + debounce = click at the stick's own force.
- **Calibration:** guided sweep (center, 8 points, full circle) in the web UI fits the
  field-to-angle map; stored in NVS with a checksum; `cal_valid` false (and HID centered) if
  the map is missing or the field magnitude leaves the calibrated envelope.
- **Tare rules:** manual anytime (button/web). Automatic only at boot, only if the stick has
  been within 2% of stored center for 3 s continuously. The drive build disables auto-tare
  entirely; silent re-centering while driving is the unacceptable failure.

---

## Processing pipeline

Order is load-bearing:

1. Calibration map: field vector -> (theta_x, theta_y, z)
2. Mount-orientation tare (gravity bias, per profile)
3. Axis rotation + invert flags
4. **Radial** deadzone (preserves diagonals)
5. Response curve LUT (linear / expo / log / S-curve), separate curves per mode
6. **1-euro filter** (tremor: heavy smoothing near rest, low lag at speed) + optional 3-12 Hz
   biquad notch for intention tremor
7. Slew limiter (optional; default-on in the drive build)
8. 16-bit quantize with dither

```cpp
// as_pipeline/include/pipeline.h
class Pipeline {
public:
    void        set_profile(const Profile& p);   // hot-swappable, lock-free
    StickSample process(const StickSample& raw);
};
```

The v1 fixed-EMA "tremor filter" is replaced by the 1-euro filter: an EMA whose cutoff rises
with input speed, so it smooths rest tremor without adding lag to deliberate motion.

---

## USB and BLE HID

**USB (TinyUSB composite, never re-enumerates on mode switch):**

| Interface | Report ID | Contents |
|-----------|-----------|----------|
| HID | 1 | Gamepad: X/Y int16, Z uint8, 16 buttons |
| HID | 2 | Mouse: 5 buttons, dx/dy int8, wheel int8 (stock TinyUSB) |
| HID | 3 | Keyboard: standard 6KRO |
| CDC | n/a | Config channel (JSON lines), WebSerial-compatible |

Mode switching changes *which reports stream*; descriptors are constant. The OS sees one stable
device, which keeps Windows happy and keeps XAC enumeration deterministic (verify in Phase 0).

**BLE (NimBLE):** one HID service exposing the same three report IDs; bonding list of 3 with a
button-combo quick switch; connection interval 7.5 ms; appearance set per active mode
(gamepad vs mouse) so mobile OSes pick sensible defaults.

**ESP-NOW:** stick streams AS-Link input frames to a paired peer MAC. The peer is either the
USB dongle (same firmware, `role=DONGLE`, presents the composite HID on the host) or an ATOS
node. ~2-4 ms link latency.

---

## Modes

- **Gamepad:** stick -> left stick; mappings (Z-click, jacks, sip/puff thresholds -> buttons,
  second-stick or trigger emulation) per profile.
- **Mouse:** deflection -> pointer velocity through the mouse curve (`max_px_s` at full
  deflection, reports at 1 kHz wired). Dwell click: pointer within `radius_pct` of rest for
  `ms` fires a click (configurable left/right/double). Scroll toggle turns the stick into a
  scroll wheel; precision toggle halves velocity.
- **Keyboard:** 8-way with directional hysteresis so diagonals do not chatter.
- **Sip/puff:** four logical buttons via thresholds (hard sip, soft sip, soft puff, hard puff,
  QuadStick convention), available as button sources in every mode.
- **ATOS input:** HID silent, AS-Link active (below).
- **Dual:** gamepad + mouse concurrently (stick aims, dwell clicks).

---

## AS-Link (ATOS integration)

Wire format mirrors ATOS `atos_drive_proposal_t` semantics (normalized axes, TTL, monotonic
seq) so the ATOS-side gateway is a thin map. Full contract in DESIGN_V2 section 9.

```c
// as_aslink/include/aslink_frame.h   (vendored into the ATOS gateway unchanged)
typedef struct __attribute__((packed)) {
    uint8_t  ver;          // 0x01
    uint8_t  type;         // 0x01 input, 0x02 health
    uint16_t seq;          // monotonic; receiver drops dupes/regressions
    int16_t  x_q15, y_q15; // [-1, +1] in Q15, post-pipeline
    int16_t  z_q15;        // press axis
    uint16_t buttons;      // jacks, sip/puff, Z-click
    uint16_t ttl_ms;       // receiver decays to zero when stale
    uint8_t  flags;        // CAL_VALID | DUAL_SENSOR_OK | TEMP_OK | TARE_STABLE
    uint8_t  reserved;
    uint32_t crc32;        // over all preceding bytes
} aslink_input_v1_t;       // 20 bytes, COBS-framed on UART (1 Mbaud) or raw in ESP-NOW
```

Plus a 1 Hz health frame (fw version, sensor disagreement stats, temperature, uptime). A
receiver that stops seeing health frames treats the input as dead regardless of input frames.

### Build profiles (Kconfig: `AS_BUILD_PROFILE`)

| Property | GAMING | DRIVE |
|----------|--------|-------|
| Dual-sensor cross-check | average + log | mandatory, latched fault on disagreement |
| Auto-tare | boot, idle-conditional | disabled (manual only while ATOS reports drive disabled) |
| Boot output | live immediately | zero until 500 ms observed neutral |
| Slew limiter | per profile | default-on |
| WiFi / BLE | as configured | WiFi off; BLE off in wired mode |
| Watchdog | task WDT | task WDT + health-frame contract |

Driving is a different integrity domain: same hardware, separate build, and the ATOS host's
limit governor remains the final authority either way. AS-Link carries intent, not stop
authority.

---

## Configuration

- **Profiles:** JSON documents in NVS/LittleFS; switch via button combo, jack, or web UI.
- **Web UI:** hold both buttons at boot -> WiFi AP `AlphaStick` -> live stick visualization,
  guided calibration sweep, curve editor, profile manager, OTA upload.
- **WebSerial:** the same JSON config protocol over USB CDC, for no-WiFi setups and scripting.
- Schema is shared with Project Nimbus where concepts overlap (curves, deadzone, profiles).

```json
{
  "name": "default",
  "mode": "gamepad",
  "mount": { "rotation_deg": 0, "gravity_tare": [0.0, 0.0] },
  "deadzone": { "type": "radial", "inner": 0.04, "outer": 0.98 },
  "curve": { "type": "expo", "gain": 1.0, "expo": 0.4 },
  "filter": { "one_euro": { "min_cutoff": 1.0, "beta": 0.02 }, "notch_hz": 0 },
  "mouse": { "max_px_s": 900, "dwell": { "enabled": true, "radius_pct": 2, "ms": 600 } },
  "buttons": { "jack1": "A", "jack2": "B", "zclick": "LMB",
               "sip_hard": "RMB", "puff_hard": "LMB" }
}
```

---

## OTA

- Primary: `esp_https_ota` against GitHub release artifacts, A/B partitions, automatic
  rollback (`esp_ota_mark_app_valid_cancel_rollback` only after a post-boot self-test:
  sensors respond, calibration loads, USB enumerates).
- Fallback: firmware upload in the web UI (config mode).
- The drive build does not self-update; updates happen deliberately, on the bench.

---

## Building and flashing

Prerequisites: [ESP-IDF 5.x](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html)
via the Espressif Windows installer, then use an "ESP-IDF PowerShell" terminal.

```powershell
# First time
cd C:\Users\Owen\dev\alpha-stick\firmware; idf.py set-target esp32s3

# Build
idf.py build

# Flash + monitor (adjust COM port; native USB port works for both)
idf.py -p COM5 flash monitor

# Configuration menu (build profile, role, log levels)
idf.py menuconfig

# Build the web UI into the LittleFS image and flash it
idf.py littlefs-flash
```

If flashing fails on the native USB port, hold BOOT while plugging in (download mode), flash,
then power-cycle.

---

## Debugging

| Issue | Likely cause | Fix |
|-------|--------------|-----|
| Device not enumerating | descriptor error, bad cable | `idf.py monitor` for TinyUSB logs; try a known-data cable |
| Axes noisy | gap too large, averaging off, magnet not diametric | re-run cal sweep; check pod assembly against HARDWARE.md targets |
| Center drifts | mount tare stale, heavy topper on tilted mount | re-tare with the profile's mount orientation; check moving-mass budget |
| `cal_valid` false | calibration missing or field out of envelope | run guided sweep; check magnet seated |
| BLE will not pair | stale bonds | clear-bonds button combo, re-pair |
| Dual-sensor fault (drive) | real disagreement or one sensor failed | health frame stats identify which sensor; do not bypass |
| Web UI missing | LittleFS image not flashed | `idf.py littlefs-flash` |

---

## Reference: Joypad OS

[Joypad OS](https://github.com/joypad-ai/joypad-os) remains the architectural reference for
modular input -> processing -> output separation and multi-protocol routing. V2's mode router
and per-mode mapping tables follow the same shape; if Alpha Stick ever grows console protocol
translation, that is the codebase to study first.

---

## Roadmap hooks

- `as_modes/` reserves an `active_force` interface for Stage C (PCB-coil force feedback):
  the force controller will be a second 1 kHz consumer of the same sensing mailbox.
- `as_aslink/` reserves frame `type` values for future bidirectional config (ATOS-side
  profile push).

See [TODO.md](../TODO.md) Phase 0 for the firmware spikes that de-risk this design.
