# Alpha Stick V2: First-Principles Redesign

Status: **Design baseline, June 2026.** Supersedes the joystick-selection sections of
[HARDWARE.md](HARDWARE.md) and the load-cell approach in [ISOMETRIC_PARTS.md](ISOMETRIC_PARTS.md)
(both kept as reference). Firmware direction changes from Arduino/PlatformIO to ESP-IDF (section 8).

---

## 0. Thesis

Build one ultra-light input core (a contactless stick on a compliant flexure pivot requiring under
5 grams of force and about 2 mm of travel) and let firmware decide what it is: a USB/BLE gamepad,
a precision mouse, a keyboard, or a drive-input node for ATOS. The expensive part of every
adaptive controller is the input mechanism. Solve that once, at the physics level, and the rest
is software.

### Decisions at a glance

| Decision | Choice | Why | Alternative kept |
|---|---|---|---|
| Sensing | Dual TMAG5273 3D Hall, one tilting magnet | Contactless (zero added force), redundancy for drive use, ~$2 | MLX90393 if bench noise is too high |
| Return force | Elastic return built into the Tetra II flexure (no spring, no centering magnet) | The restoring force *is* the pivot; zero sliding friction, zero backlash | Active coils (Stage C); ball + magnet centering (alternative) |
| Pivot | Tetra II spherical flexure, printed one piece | No bearings, no sliding surfaces, no break-away friction; remote centre ~50 mm out | Ball in PTFE cup + magnet preload if printed-flexure creep/fatigue loses on the bench |
| Throw | +/-4 to 6 degrees (~+/-2-3 mm at fingertip) | Proprioceptive feedback without fatigue; well within the flexure's range | Stiffer-blade flexure or firmware near-isometric mode |
| Isometric load cells | **Dropped** | Sustained force is fatiguing; strain gauges drift; 4 cells + 2 ADCs + mux for worse UX | Docs archived |
| MCU | ESP32-S3 (MINI-1 module) | Native USB OTG + BLE + WiFi, same target as ATOS | ESP32-C3 cannot do USB HID composite well |
| Framework | ESP-IDF 5.x + TinyUSB + NimBLE | Composite HID, OTA rollback, ESP-NOW, shares toolchain with ATOS/espp | Arduino fine for quick sensor spikes |
| Mouse vs joystick | Both, always: composite HID device, runtime mode switch | No re-enumeration, one firmware | |
| ATOS link | "AS-Link" framed protocol (UART or ESP-NOW), mirrors ATOS drive-proposal semantics | ATOS host keeps final authority; stick is a sensor, never a motor controller | USB HID into Layer 2 Linux |

---

## 1. Requirements (with numbers)

### Hard requirements

| # | Requirement | Target | Notes |
|---|---|---|---|
| R1 | Full-deflection force | <5 gf, nominal ~3 gf target; set by flexure blade geometry/scale, **measured** on the bench (not per-session adjustable) | Measured at 40 mm reference grip point |
| R2 | Throw | +/-4 to 6 deg mechanical (+/-2.8 to 4.2 mm at 40 mm) | "Super low throw" |
| R3 | Breakout (force to first motion) | <0.5 gf (~0 with the flexure: no sliding surface to break away) | Friction budget, section 3 |
| R4 | Sensing | Contactless (Hall effect) | No pots, no strain gauges in the force path |
| R5 | MCU | ESP32-S3 | USB HID + BLE HID + WiFi config/OTA |
| R6 | Modes | Gamepad, mouse, keyboard, ATOS input | Runtime switchable |
| R7 | Wired latency, stick to USB report | <5 ms | Section 8 budget |
| R8 | Effective resolution | >=8 bits per axis at 1 kHz, >=9 after filtering | Comparable to commercial thumbsticks |
| R9 | Core BOM | <$35 wired, <$50 with battery | Project goal |
| R10 | Buildable | FDM printer + soldering iron, no machining | Project goal |

### Soft requirements

- Z-axis press-to-click at <5 gf (same Hall sensor, no switch in the force path)
- 4x 3.5mm switch jacks (XAC-style ecosystem)
- Sip/puff option
- Battery option, 10+ hours BLE
- Sensor pod usable in multiple bodies: desktop base, chin boom, mouth-stick frame, thumb pad

---

## 2. Why neither v1 option survives first principles

**Option A, gimbal potentiometer sticks (KY-023, PSP, even Hall RKJXV):** centering springs on
these parts sit at 150-450 gf full deflection, two orders of magnitude above target. Swapping
springs does not fix it: the gimbal has sliding friction at four contact lines, and at the 3 gf
scale that friction becomes the dominant feel (stiction notches, hysteresis, no clean return to
center). The architecture is wrong for the force target, not just the spring.

**Option B, isometric load cells (v1's accessibility path):** zero motion sounds ideal, but:

1. **Sustained force is the fatigue mode.** Holding a direction means holding a force forever.
   For muscular dystrophy and similar conditions, a tiny displacement that can be *rested at*
   beats a force that must be *maintained*.
2. **No proprioception.** Nothing moves, so the only feedback is on-screen. Tremor and overshoot
   get worse without position feedback.
3. **Strain gauges drift** with temperature and creep, so center wanders and needs re-taring,
   which is exactly what you cannot tolerate while driving a cursor or a wheelchair.
4. **Complexity:** 4 cells + 2 precision ADCs + an I2C mux to do badly what one $1 Hall sensor
   does well.

**The synthesis:** a *small-displacement* stick (2-3 mm) at *near-zero force* (3 gf) gives the
minimal range-of-motion benefit that motivated the isometric design, plus real proprioceptive
feedback, plus rest-at-position, with one contactless sensor. Users who want a stiffer, almost
isometric feel fit a stiffer-blade flexure; the throw is small enough that ~8 gf feels nearly rigid.

---

## 3. The force budget (where every gram goes)

Force at the 40 mm grip point, full deflection:

| Source | Budget | How |
|---|---|---|
| Flexure restoring force (the intended force) | nominal ~3.0 gf, fixed by the part | The flexure blades bend elastically; force is set by blade geometry/material/scale and measured on the bench (section 5) |
| Pivot friction | ~0 gf | The flexure has no sliding surfaces, so no break-away friction and no stiction |
| Seal/boot | 0 (default) | Labyrinth gap, no contacting seal. Optional silicone boot costs 0.5-1 gf, off by default |
| Sensor | 0 | Contactless |
| Gravity bias (tilted mounts) | up to ~1.3 gf equivalent | See below; handled by mass budget + firmware tare |

**Gravity is a real term at this scale.** A stick with moving mass m and center of gravity h
above the pivot, mounted horizontally (chin boom, vertical surface), sees a constant gravity
torque of m * g * h. With m = 2.5 g and h = 20 mm that is 50 gf-mm, i.e. ~1.25 gf at the 40 mm
grip point, nearly half the nominal centering force. Therefore:

- **Moving-mass budget: <2.5 g total** (stick tube + topper + magnet + ball). Carbon tube,
  hollow printed toppers, one small magnet.
- Firmware applies a **mount-orientation tare** (stored per profile) so the gravity bias reads
  as zero offset.
- Tilted mounts (chin boom, vertical surface) use a stiffer-blade flexure so the stick still
  returns to center against gravity. The flexure's moving platform adds to the moving mass, so
  re-derive this budget against the measured part rather than the ball-pivot table above.

This is also why the topper system is magnetic quick-swap with a <1 g mass budget per topper.

---

## 4. Sensing: one tilting magnet, two 3D Hall sensors

### Geometry

```
                topper (magnetic quick-swap, <1 g)
                  |
                  |   stick: carbon tube, 25/40/60 mm options
                  |
              [ P ]            remote rotation centre, ~50 mm out
                  :
        ##########:##########   Tetra II spherical flexure (printed one piece):
        ##  nested blade   ##   3-DOF rotation about P, no sliding surfaces,
        #####moving########      elastic return to centre built in
             [N52 D4x2]          diametric disc magnet on the moving platform,
                  |              tilts 1:1 with stick angle
         . . . . gap ~1.5 mm . . . .
       =============================    sensor pod PCB (~20 x 20 mm)
        [TMAG5273 A]   [TMAG5273 B]     dual 3D Hall, I2C, ~3 mm apart
       =============================    flexure base mounts here
```

### Why field *direction*, not magnitude

The magnet tilts with the stick, so the field vector at the sensor rotates 1:1 with stick angle.
Computing tilt from the field direction (after a one-time calibration map) and normalizing by
|B| makes the measurement first-order immune to:

- NdFeB temperature coefficient (~ -0.12 %/degC of magnitude, direction unaffected)
- Axial gap creep in printed parts (magnitude changes, direction does not)
- Magnet strength tolerance between builds

The only ferromagnetic part is the sense magnet itself (the flexure is all-polymer), so there is
little to distort the field, and the calibration map absorbs whatever static asymmetry the build has.

### The Z axis (press-to-click)

A spherical flexure constrains the three translations, so axial press is *stiff* — unlike the ball
pod, pressing does not freely close the magnet-sensor gap. Z press-to-click is therefore a
follow-up on the flexure: either a deliberately compliant axial element under the pod (a soft
mount that lets |B| dip a little under press, read with hysteresis + debounce, no switch in the
force path) or a jack/topper switch. The ball-pivot alternative keeps the free gap-change Z-click.

### Dual sensors: cheap redundancy

Two TMAG5273 (two of the four factory I2C address variants) read the same magnet:

- **Gaming build:** average them (small noise win) or run one and keep the second dark.
- **Drive build (ATOS):** continuous plausibility cross-check. Disagreement >3% of full scale
  for >50 ms latches a fault and zeroes output (section 9). This is the cheapest path to a
  drive-grade input: ~$1 of silicon, no exotic parts.

### Expected performance (validate on bench, Phase 0)

| Parameter | Estimate | Basis |
|---|---|---|
| Field swing per axis | ~ +/-3.5-8 mT | |B| 40-100 mT, +/-5 deg rotation |
| Sensor noise after 8x averaging | tens of uT rms | TMAG5273 datasheet class |
| Effective resolution at 1 kHz | 8-9 bits/axis | swing / noise; dual-sensor and pipeline filtering add margin |
| Sample rate | 1 kHz output (sensor ~3 kSPS raw) | 25 us/channel conversions |

8-9 effective bits matches or beats typical commercial thumbsticks. If bench results disappoint,
the drop-in upgrade is MLX90393 (16-bit, configurable oversampling) on the same pod footprint.
HID reports are 16-bit with dithered scaling either way.

### Calibration

1. **Factory/build cal:** guided sweep in the web UI (center, 8 compass points, full circle)
   fits a small polynomial map from field vector to (theta_x, theta_y, z). Stored in NVS.
2. **Center tare:** on demand (button/web UI) and automatically at boot *only if* the stick has
   been continuously still and within 2% of stored center for 3 s. Never silently re-tares while
   in use: silent re-centering is how drift bugs eat trust, and in drive mode it is dangerous
   (drive build disables auto-tare entirely).

---

## 5. Mechanics

### Pivot trade study

| Option | Friction | Wear | Printable | Verdict |
|---|---|---|---|---|
| 2-axis gimbal | 4 sliding contacts | yes | hard | No: friction dominates at 3 gf |
| **Tetra II spherical flexure** | zero (no sliding) | creep/fatigue (polymer) | yes, one piece | **Chosen.** No break-away friction, no backlash, self-centering; creep/fatigue is the accepted risk, bounded by material (PETG/metal) and the ball-pivot fallback |
| Ball in PTFE cup + magnetic preload | ~0.1-0.2 gf equiv | low (PTFE) | yes | Alternative / fallback; also characterises the sensing on the bench independent of the pivot |
| Jewel/pivot bearing | near zero | low | no ($) | Fallback if both printed paths disappoint |

The flexure was the rejected option in the V1-era version of this study — printed-polymer creep
moving centre over weeks is a real failure mode. It is promoted to primary now because its
bench-validatable upside (literally zero break-away friction, the first gram of input already
moves it, plus self-centering with no extra parts) is exactly the ultra-low-force thesis, and the
creep/fatigue risk is bounded: PETG over PLA, a printed-then-cast/metal path, and the ball-pivot
pod kept as a drop-in fallback if the bench says creep wins.

### Centering and force: the flexure does both

The Tetra II flexure's blades bend elastically, so the joint *is* the return-to-centre spring —
no centering magnet, no ferrous washer, no threaded carrier. Required restoring torque is tiny
(3 gf at 40 mm = 120 gf-mm = ~1.2 mN-m), and the flexure delivers it with zero sliding friction
and zero backlash.

Force is therefore a property of the printed part, not a user dial. It is set by blade thickness,
the nested-element geometry, material modulus, and overall scale — and the paper's closed-form
stiffness matches the real tetrahedron element only qualitatively (NMAE ~35%), so **measure it on
the bench** (Phase 0) rather than trusting a calculation. To retune feel, print a flexure with
different blade thickness or a scaled STEP (scaling moves P and stiffens the blades non-linearly —
re-measure). Per-session, tool-free force adjustment is the one thing given up versus the
magnetic-carrier design: it returns as the Stage C active-coil feature below, and the ball-pivot
alternative keeps the threaded adjuster for builds that need it.

### On magnetic levitation (asked and answered)

Earnshaw's theorem rules out stable levitation with static permanent magnets alone; every
"floating" stick needs either a mechanical constraint or active control. So:

- **Now (Stage A):** *constrained passive compliance*: the flexure carries both the constraint
  and the restoring force, with no sliding surfaces and no centering magnet. Zero break-away
  friction, zero backlash; the wear mode is polymer creep/fatigue, bounded by material choice.
  This captures ~90% of what maglev promises at ~0% of the complexity.
- **Later (Stage C, "Field"):** *active magnetics*: four planar coils in the sensor-pod PCB
  layers push on the stick magnet, closed-loop at 1 kHz from the Hall sensors. That unlocks
  firmware-programmable centering force (0 to ~10 gf), virtual detents and walls, haptic ticks,
  and **active tremor damping** (velocity-proportional opposing force at the physical layer, not
  just filtering). Torque needed is ~1.2 mN-m, which is plausible for PCB coils at this gap but
  must be proven; Stage C begins with a single go/no-go bench test of coil force. Power budget
  ~100-300 mW when active, so it is a wired/dock feature first.

No commercial adaptive joystick offers a programmable force field. Stage C is the "next level"
bet; Stages A and B do not depend on it.

### The sensor pod: one core, many bodies

Everything input-critical lives in a self-contained **pod**: flexure pivot + magnet + dual Hall
on a ~20 x 20 mm satellite PCB, connected to the main board by a 6-pin JST-SH/FFC
(3V3, GND, SDA, SCL, INT_A, INT_B). The pod mounts in:

- **Desktop base** (default): main PCB, jacks, USB-C, battery
- **Chin boom pod**: pod + cable only, electronics stay on the chair rail
- **Mouth-stick frame**: pod behind a food-safe mouthpiece adapter (QuadStick-style use)
- **Thumb pad**: pod in a 30 mm puck for tray mounting

One calibration procedure, one firmware, four form factors.

### Toppers and mounting

- Toppers: magnetic quick-swap, <1 g each: 8 mm ball, 14 mm dish, lip bar, T-bar,
  20/40 mm extensions, mouthpiece adapter
- Base: AMPS hole pattern + 1/4-20 brass insert (RAM mounts, Magic Arms, wheelchair trays)
- Printed in PLA/PETG; the flexure prints as one piece (PETG preferred over PLA for creep and
  fatigue life). The ball-pivot alternative uses a PTFE cup (machined insert or printed PCTG +
  PTFE tape)

---

## 6. Electronics

### Block diagram

```
                       +--------------------------------------+
   sensor pod          |              main board              |
  +-------------+      |                                      |
  | TMAG5273 x2 |--I2C-|  ESP32-S3-MINI-1                     |
  | magnet+flex |      |   |- USB OTG --- USB-C ---> PC/XAC   |
  +-------------+      |   |- BLE HID ------------> wireless  |
                       |   |- ESP-NOW ----> dongle / ATOS S3  |
  4x 3.5mm jacks ------|   |- UART (AS-Link) -----> ATOS S3   |
  2x tactile ----------|   |- WiFi AP ---> web config + OTA   |
  sip/puff (opt) --I2C-|                                      |
                       |  BQ24074 charger + MAX17048 gauge    |
                       |  (battery option)   WS2812  DRV2605L |
                       +--------------------------------------+
```

### Parts

| Block | Part | Why |
|---|---|---|
| MCU | ESP32-S3-MINI-1-N8 | Native USB OTG (TinyUSB), BLE 5, WiFi, same target/toolchain as ATOS |
| Hall sensors | 2x TMAG5273A1 (different address variants) | 3-axis, I2C, ~$0.80 each |
| Charger | BQ24074 | Power-path: runs while charging, no TP4056 brownout dance |
| Fuel gauge | MAX17048 | Real % instead of voltage guessing |
| Battery | LiPo 503035 500 mAh (optional) | 10-20 h BLE estimate, validate |
| Regulator | RT9080-33 class LDO | Low IQ |
| USB | USB-C 16-pin, CC resistors, USBLC6-2SC6 ESD | Proper device behavior |
| Jacks | 4x PJ-320A 3.5 mm mono + RC debounce + ESD | XAC-style switch ecosystem |
| Sip/puff (opt) | MPXV7002DP (analog, matches v1 docs) or Honeywell ABP I2C | +/-2 kPa differential fits sip/puff pressures |
| Haptics (opt) | LRA + DRV2605L | Mode-change/click confirmation ticks |
| Status | 2x WS2812B-2020 | Mode, battery, pairing |
| Expansion | 1x Qwiic/STEMMA-QT | Future sensors without respins |

Keep the WS2812 and any speaker magnets >15 mm from the sensor pod; static fields calibrate out,
but switching currents near the Hall sensors do not.

### Pin map (main board, draft)

| Function | GPIO | Notes |
|---|---|---|
| I2C0 SDA/SCL (pod + gauge + opt sensors) | 8 / 9 | 400 kHz-1 MHz |
| Pod INT_A / INT_B | 10 / 11 | Conversion-ready |
| Jacks 1-4 | 4 / 5 / 6 / 7 | Pullups, RC, ESD |
| Buttons 1-2 | 12 / 13 | |
| WS2812 | 48 | |
| Haptic EN | 14 | DRV2605L on I2C0 |
| Sip/puff ADC | 1 | ADC1_CH0 (analog option) |
| UART AS-Link TX/RX | 17 / 18 | To ATOS host |
| USB D-/D+ | 19 / 20 | Fixed |

(Avoids strapping pins 0/3/45/46. Final assignment at schematic time.)

### Core BOM estimate (wired build)

| Item | Cost |
|---|---|
| ESP32-S3-MINI-1-N8 | $3.50 |
| 2x TMAG5273 | $1.60 |
| Magnets (sense + toppers; no ring) | $2.00 |
| USB-C + ESD + regulator + passives | $2.50 |
| 4x 3.5 mm jacks | $1.60 |
| Buttons, LEDs, misc | $1.50 |
| PCBs (main + pod, JLC) | $4.00 |
| Brass/nylon fasteners (flexure is printed) | $0.50 |
| Printed parts (incl. Tetra II flexure) | $3.50 |
| **Core total** | **~$20** |
| Battery option (LiPo + BQ24074 + MAX17048) | +$13 |
| Sip/puff option | +$10 |
| Haptic option | +$4 |
| ESP-NOW dongle (minimal second S3 board) | +$9 |

Within R9 with margin.

---

## 7. What it acts as: modes

All USB modes live in **one composite HID device** (gamepad + mouse + keyboard report IDs, plus
CDC for config). Mode switching changes which reports stream; the device never re-enumerates.
BLE exposes the same report IDs in one HID service.

| Mode | Behavior |
|---|---|
| **Gamepad** | X/Y -> left stick (16-bit), Z-press + jacks + buttons -> buttons; second-stick and trigger mappings per profile |
| **Mouse** | Deflection -> pointer velocity through its own curve; Z-press = left click; jacks = right/middle/back; **dwell click** (configurable radius + time) for users who cannot click; scroll mode toggle; precision (half-speed) toggle |
| **Keyboard** | 8-way WASD/arrows with hysteresis, for menu UIs and games without stick support |
| **ATOS input** | AS-Link frames out UART/ESP-NOW, HID silent (section 9) |
| **Dual** | Gamepad + mouse simultaneously (e.g. stick aims, dwell clicks) |

### Platform reach (honest version)

| Platform | Path | Confidence |
|---|---|---|
| PC (Win/Linux/macOS) | USB composite HID / BLE HID | High |
| Xbox | **Via XAC USB port** as generic HID stick (X1/X2) | Medium-high; verify against XAC's supported-device behavior in Phase 0 |
| PlayStation | Via PS Access **3.5 mm AUX ports** for switch-type signals; analog passthrough not assumed | Medium |
| Switch | BLE Pro-Controller emulation (community-proven approach) | Medium |
| Android/iOS | BLE HID gamepad/mouse | High |
| Project Nimbus | Appears as standard HID; Nimbus layers on-screen controls; share curve/profile JSON schema | High |

Native XInput/console-auth modes are explicitly out of scope (auth chips); the XAC and Access
controllers are the sanctioned adaptive paths and we design to them.

---

## 8. Firmware (ESP-IDF)

### Why ESP-IDF now

TinyUSB composite control, NimBLE multi-report HID, ESP-NOW, and OTA-with-rollback are all
first-class in IDF and awkward in Arduino-on-S3. ATOS is ESP-IDF + espp on the same chip, so
components (logging, task wrappers, filters) and toolchain knowledge transfer directly.
Arduino remains fine for throwaway sensor bring-up spikes.

### Task graph

```
 sensor_task (core 1, 1 kHz, highest prio)
   read both TMAG5273 -> cross-check -> calibration map -> pipeline -> mailbox
 router_task (core 0)
   mailbox -> active mode mapping -> usb_hid / ble_hid / aslink writers
 ui_task        LEDs, haptics, button combos, force-adjuster hints
 config_task    WiFi AP + web UI + WebSerial (config mode only)
 watchdog       feeds only if sensor_task meets deadline and data is fresh
```

Latest-value seqlock mailbox between producer and writers (same pattern as ATOS's clamped
setpoint mailbox): writers never block the 1 kHz loop.

### Processing pipeline (order matters)

1. Calibration map (field vector -> theta_x, theta_y, z)
2. Mount-orientation tare (gravity bias)
3. Axis rotation (mounting angle) + invert flags
4. **Radial** deadzone (preserves diagonals, unlike per-axis)
5. Response curve LUT (linear/expo/log/S, per profile, separate curves for gamepad vs mouse)
6. **1-euro filter** for tremor (low lag at speed, heavy smoothing near rest; strictly better
   than the v1 fixed EMA), plus optional 3-12 Hz biquad notch for intention tremor
7. Slew limiter (optional, drive mode default-on)
8. 16-bit quantize with dither

### Latency budget (R7)

| Stage | Budget |
|---|---|
| Sensor conversion + averaging | ~0.6 ms |
| Pipeline | <0.1 ms |
| USB FS HID poll | 1 ms |
| **Wired total** | **~2-3 ms** |
| BLE (7.5 ms conn interval) | ~9-15 ms |
| ESP-NOW -> dongle -> USB | ~3-5 ms |

### Config and updates

- Profiles: JSON in NVS/LittleFS; curve/deadzone/filter/mapping/mount-tare per profile;
  hold-combo or jack to switch; shared schema with Project Nimbus
- Web UI over WiFi AP (live stick visualization, guided calibration, curve editor) plus
  WebSerial/WebHID over the CDC interface for no-WiFi config
- OTA: esp_https_ota with A/B partitions and automatic rollback; web upload kept as fallback

---

## 9. ATOS integration (the next level)

ATOS's Layer 1 whitepaper designs input-device integration but has not built it, and its host
owns the joystick and the final motor write. Alpha Stick v2 slots into exactly that hole: it is
a **high-integrity input sensor node**, never a motor controller.

### Link options

| Link | Use |
|---|---|
| UART, COBS-framed, 1 Mbaud | Wired on-chair, deterministic, preferred for drive |
| ESP-NOW direct to the ATOS S3 | Wireless on-chair, ~2-4 ms, no pairing UX |
| USB HID into Layer 2 (Linux) | Pointer/UI control of the chair's compute layer, never drive |

### AS-Link frame (v1 draft)

Semantics deliberately mirror `atos_drive_proposal_t` (normalized axes, TTL, monotonic seq) so
the ATOS-side gateway is a thin map:

```c
struct aslink_input_v1 {        // little-endian, COBS-framed, 20 bytes
    uint8_t  ver;               // 0x01
    uint8_t  type;              // 0x01 input, 0x02 health
    uint16_t seq;               // monotonic, host drops dupes/regressions
    int16_t  x_q15, y_q15;      // normalized [-1, +1] in Q15, post-pipeline
    int16_t  z_q15;             // press axis
    uint16_t buttons;           // jacks, sip/puff thresholds, Z-click
    uint16_t ttl_ms;            // host decays output to zero when stale
    uint8_t  flags;             // CAL_VALID | DUAL_SENSOR_OK | TEMP_OK | TARE_STABLE
    uint8_t  reserved;
    uint32_t crc32;
};                              // + 1 Hz health frame: fw version, sensor
                                //   disagreement stats, temperature, uptime
```

ATOS side: an `alpha-stick-gateway` module at `ATOS_LEVEL_L3_COMMAND` with
`ATOS_CAP_PROPOSE_DRIVE | ATOS_CAP_READ_TELEMETRY | ATOS_CAP_LOGGING`, mapping x/y to
linear/angular with ATOS-side curves and submitting proposals the host clamps as usual. The ABI
needs one addition (host owns the UART): a `read_input_frame()` host service behind a new
`ATOS_CAP_READ_INPUT_STREAM` capability, consistent with the whitepaper's input-router design.

### Drive build vs gaming build

Driving a wheelchair is a different integrity domain than gaming. Same hardware, **separate
firmware build profile**, honest about the differences:

| Property | Gaming build | Drive build |
|---|---|---|
| Dual-sensor cross-check | optional averaging | **mandatory**; >3% FS disagreement >50 ms latches fault, output zero, recover only at neutral + explicit ack |
| Auto-tare | idle-conditional | **disabled**; manual tare only while ATOS reports drive_disabled |
| Stale behavior | last value briefly held | TTL decay to zero (host side, AS-Link contract) |
| Boot behavior | live immediately | output held zero until stick observed neutral 500 ms |
| Tremor filter | user preference | slew limiter default-on; ATOS host applies its own envelope regardless |
| Watchdog | task WDT | task WDT + health frame; missing health = host treats input as dead |
| Radio | BLE/WiFi as configured | WiFi off; BLE off in wired mode |

Design posture aligns with ISO 7176-14 (power wheelchair control systems) but **no compliance is
claimed**: Alpha Stick proposes, the ATOS host's limit governor and safe-state policy dispose.
E-stop remains an ATOS-side physical input; AS-Link carries no stop authority, only intent.

---

## 10. Staged roadmap

### Stage A: "Glide" (the core product)

Passive pod (Tetra II flexure pivot + dual Hall sensing), desktop base, USB composite
gamepad/mouse/keyboard, BLE HID, web config, profiles, OTA.

**Exit criteria (bench-measured, not vibes):**
- Flexure full-deflection force measured at 40 mm (pull gauge); a printed flexure landing near the ~3 gf nominal target, with a stiffer-blade variant on hand for tilted mounts
- Breakout <0.5 gf (expected ~0); release returns within 0.5% FS of center
- Stationary noise <0.5% FS p-p after pipeline; >=8 effective bits demonstrated
- Wired stimulus-to-report latency <5 ms (logic analyzer)
- Center drift <1% FS over 2 h, 15-35 degC
- 100k-cycle deflection + soak rig: bound flexure creep/fatigue — force and centre drift stay within spec, or change material/scale (PETG/metal) until they do
- Owen daily-drives it for gaming and desktop mousing (the real test)

### Stage B: "Bridge" (the ecosystem release)

3.5 mm jacks, sip/puff, chin/mouth/thumb pod bodies, ESP-NOW dongle, AS-Link over UART +
ATOS gateway module prototype, XAC/PS-Access/Switch verification, Nimbus profile schema,
battery option, kit documentation.

### Stage C: "Field" (the research bet)

PCB-coil active stage: go/no-go force bench first (can four planar coils produce ~1.2 mN-m at
the working gap?), then programmable centering, virtual detents, haptic ticks, active tremor
damping. Wired/docked feature first; falls back cleanly to Stage A passive behavior.

---

## 11. Risks and open questions

| Risk | Mitigation |
|---|---|
| Hall SNR below 8 effective bits at +/-5 deg | First Phase 0 bench item; MLX90393 fallback footprint on pod v0 |
| Printed-flexure creep/fatigue moves centre or changes force over weeks | The accepted primary risk: 100k-cycle + soak bench tests; PETG over PLA, then printed-then-cast/metal; ball-pivot pod is the drop-in fallback |
| Flexure force not predictable from the paper (NMAE ~35%) and not per-session adjustable | Measure on the bench, iterate blade thickness/scale; per-session tuning deferred to Stage C coils; ball-pivot adjuster as alternative |
| Flexure remote centre (~50 mm) and envelope (~111 x 103 x 45 mm) are large for a handheld stick | Scale the STEP down (re-measure force; scaling moves P non-linearly); mount the stick so its natural pivot sits at P |
| S3 BLE power too high for 10 h on 500 mAh | Measure; modem sleep tuning; bigger cell fits the bay |
| XAC rejects generic HID stick | Test early (Phase 0); fallback is XAC 3.5 mm for buttons + USB on PC only |
| ATOS ABI lacks input-stream service | Filed as proposed `ATOS_CAP_READ_INPUT_STREAM`; gateway can prototype against a host stub |
| Moving mass creeps over 2.5 g with toppers | Mass column in topper README; chin/vertical profiles default stiffer |

---

## 12. Relationship to existing docs

- [HARDWARE.md](HARDWARE.md): rewritten to this baseline; it is the build reference (parts,
  pins, wiring, BOM), while this document holds the rationale.
- [FIRMWARE.md](FIRMWARE.md): rewritten to this baseline (ESP-IDF, task graph, HID, AS-Link).
- [ISOMETRIC_PARTS.md](ISOMETRIC_PARTS.md): archived as an alternative path; the rationale for
  dropping it is section 2.
- ATOS references: `docs/whitepaper-atos-layer1-modules.md` and `docs/display-casting.md` in the
  ATOS repo, plus `atos_module_abi.h` for the descriptor/capability/proposal contract.
