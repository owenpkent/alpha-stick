# Alpha Stick TODO

Task tracking for the Alpha Stick project.

---

## Phase 0: V2 Redesign Validation (Current)

First-principles redesign baseline: see [docs/DESIGN_V2.md](docs/DESIGN_V2.md). Bench-validate the core physics before any enclosure or PCB work.

### Order
- [ ] **Order Phase 0 bench parts** per [docs/PHASE0_PARTS.md](docs/PHASE0_PARTS.md)

### Sensing Bench
- [ ] **Breadboard dual TMAG5273 + tilting magnet** and measure field swing, noise, effective bits at 1 kHz
- [ ] **Decide TMAG5273 vs MLX90393** from bench data (need >=8 effective bits/axis)
- [ ] **Verify Z-press detection** (gap change) with clean hysteresis at <5 gf

### Mechanics Bench
- [x] **Model pod v0 printables** — parametric OpenSCAD source + STLs ([models/](models/))
- [ ] **Prototype magnetic centering + threaded force adjuster**, confirm 1-8 gf range with gram gauge
- [ ] **Measure ball-in-PTFE pivot breakout force** (<0.5 gf target) and return-to-center repeatability
- [ ] **Print sensor pod v0** (pivot + magnet + pod PCB outline + adjuster carrier)
- [ ] **Weigh moving assembly** against the 2.5 g budget

### Firmware Spikes
- [ ] **ESP-IDF TinyUSB composite spike** (gamepad + mouse + CDC) at 1 kHz on S3 devkit
- [ ] **BLE HID multi-report spike** (gamepad + mouse in one HID service)
- [ ] **Measure wired stimulus-to-report latency** (<5 ms target)

### Integration Groundwork
- [ ] **Draft AS-Link frame header** (shared .h) and an ATOS gateway module skeleton
- [ ] **Test a generic HID stick against the XAC USB port** to de-risk the Xbox path
- [x] **Rewrite HARDWARE.md and FIRMWARE.md** to match the V2 baseline

---

## Phase 1: Foundation (largely superseded by Phase 0, see docs/DESIGN_V2.md)

### Hardware Design
- [ ] **Research joystick modules** — Compare analog vs hall-effect options
- [ ] **Select ESP32 variant** — ESP32-S3 vs ESP32-C3 for USB HID support
- [ ] **Define 3.5mm jack pinout** — Standard for external switch compatibility
- [ ] **Create initial schematic** — Basic connections for MVP
- [ ] **Identify mounting options** — RAM mount, Magic Arm, flat surface

### 3D Modeling
- [ ] **Design base enclosure** — Fits ESP32 + joystick + 2 buttons
- [ ] **Design top cover** — Joystick access, ergonomic palm rest
- [ ] **Create mounting adapter** — Compatible with standard camera mounts
- [ ] **Prototype in CAD** — FreeCAD or Fusion360

### Firmware
- [x] **Set up ESP-IDF project** — ESP32-S3 target (`firmware/` scaffold with simulation mode)
- [ ] **Implement USB HID gamepad** — Basic joystick + buttons
- [ ] **Read analog joystick input** — ADC sampling with smoothing
- [ ] **Add basic deadzone** — Configurable center deadzone

### Documentation
- [x] Create README.md with project overview
- [x] Create TODO.md for task tracking
- [ ] Create LLM_ONBOARDING.md for AI assistants
- [ ] Draft HARDWARE.md with component selection rationale
- [ ] Draft FIRMWARE.md with architecture overview
- [ ] Draft PRINTING.md with recommended settings

---

## Phase 2: Prototype

### Hardware
- [ ] **Build breadboard prototype** — Validate component choices
- [ ] **Test joystick modules** — Compare feel and precision
- [ ] **Test 3.5mm switch input** — Verify external switch compatibility
- [ ] **Design PCB v0.1** — Single-sided, easy to solder
- [ ] **Order PCB prototype** — JLCPCB or similar

### 3D Printing
- [ ] **Print first enclosure** — Test fit, ergonomics
- [ ] **Iterate on design** — Refine based on testing
- [ ] **Add ventilation** — Prevent overheating
- [ ] **Design button caps** — Different shapes for different needs

### Firmware
- [ ] **Implement sensitivity curves** — Linear, exponential, custom
- [ ] **Add button debouncing** — Configurable timing
- [ ] **Create web configuration portal** — ESP32 soft AP mode
- [ ] **Store settings in NVS** — Persistent configuration
- [ ] **Implement profile system** — Save/load multiple configs

### Testing
- [ ] **Test with Xbox Adaptive Controller** — 3.5mm jack compatibility
- [ ] **Test with PC games** — USB HID recognition
- [ ] **Test with Project Nimbus** — Integration verification
- [ ] **Gather user feedback** — Ergonomics, usability

---

## Phase 3: Refinement

### Bluetooth Support
- [ ] **Implement BLE HID gamepad** — Wireless operation
- [ ] **Add pairing mode** — Button combo to enter pairing
- [ ] **Test multi-device pairing** — Switch between devices
- [ ] **Optimize power consumption** — For battery operation

### OTA Updates
- [ ] **Implement OTA update system** — esp_https_ota with A/B rollback
- [ ] **Create update server/process** — GitHub releases integration
- [ ] **Add rollback mechanism** — Recover from bad updates

### Advanced Features
- [ ] **Sip/puff input** — Pressure sensor integration
- [ ] **Tremor filtering** — Configurable smoothing algorithm
- [ ] **OLED status display** — Show profile, battery, connection
- [ ] **LED indicators** — Status, profile, low battery
- [ ] **Battery monitoring** — Voltage sensing, low battery warning

### Alternative Input Modes
- [ ] **Mouse mode** — Joystick controls cursor
- [ ] **Keyboard mode** — Joystick sends arrow keys
- [ ] **Xbox Adaptive mode** — Optimized for XAC compatibility
- [ ] **Flight sim mode** — Multi-axis with throttle

---

## Phase 4: Release

### Documentation
- [ ] **Complete assembly guide** — Step-by-step with photos
- [ ] **Create video tutorial** — Build walkthrough
- [ ] **Write troubleshooting guide** — Common issues and fixes
- [ ] **Translate to other languages** — Community contributions

### Community
- [ ] **Create Discord server** — Community hub
- [ ] **Build profile library** — Shared configurations
- [ ] **Publish to Makers Making Change** — Reach accessibility community
- [ ] **Submit to Hackaday.io** — Visibility in maker community

### Polish
- [ ] **Finalize PCB design** — Production-ready
- [ ] **Create BOM with links** — Easy ordering
- [ ] **Test across platforms** — PC, Xbox, PlayStation, Switch
- [ ] **Tag v1.0 release** — Stable, documented, tested

---

## Backlog / Future Ideas

- [ ] Mouth-operated variant (inspired by QuadStick)
- [ ] Foot pedal design
- [ ] Eye-tracking integration
- [ ] Voice command integration
- [ ] Haptic feedback (vibration motor)
- [ ] Modular button plates (hot-swap different layouts)
- [ ] Wireless charging support
- [ ] Integration with Steam Input
- [ ] macOS/Linux configuration app
- [ ] Mobile app for configuration

---

## Completed

- [x] Set up GitHub repository
- [x] Add MIT license
- [x] Create Constellation integration guide

---

*Last updated: January 2026*
]]>
