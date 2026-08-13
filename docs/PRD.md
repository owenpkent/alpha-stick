# Alpha Stick PRD (v0.1, first pass)

**One-liner:** An open-source, 3D-printable joystick that moves with under 5 grams of force, costs about $25 in parts, and works as a gamepad, a mouse, and (later) a wheelchair drive input.

**Owner:** Owen Kent | **Status:** Draft, Phase 0 (bench validation) | **Date:** August 2026
**Scope of this PRD:** Stage A "Glide" (the core stick). Stages B/C are out of scope, see [DESIGN_V2.md](DESIGN_V2.md) section 10.

---

## 1. Problem

A standard thumbstick needs 150 to 450 gf to deflect, which is a wall for people with muscular dystrophy, SMA, ALS, and similar conditions. Devices built for ultra-low force exist but cost $200 to $500+, are proprietary, and cannot be repaired, adapted, or replicated by the people who depend on them.

## 2. Target users

| Primary | Gamers with limited hand strength or range who can produce a few grams of force and millimeters of travel |
|---|---|
| Secondary | Makers, OTs, and assistive tech builders who fit devices to a specific person |
| Tertiary | ATOS / RAMMP integrators who need an open input device for a wheelchair safety host |

## 3. Goals and non-goals

**Goals (Stage A):** ultra-low-force stick that anyone can print and solder; plug-and-play USB and BLE HID; browser-based configuration; one stick that reaches a whole gamepad's controls via runtime layer switching; honest, bench-measured specs.

**Non-goals (this release):** wheelchair drive certification or any safety claim; active-coil force feedback (Stage C); sip/puff and 3.5 mm jacks (Stage B); a manufactured/retail product; console-native Xbox or PlayStation wireless protocols.

## 4. Requirements

| # | Requirement | Target | Priority |
|---|---|---|---|
| R1 | Full-deflection force at 40 mm grip point | <5 gf, nominal ~3 gf | Must |
| R2 | Throw | +/-4 to 6 deg (+/-2.8 to 4.2 mm) | Must |
| R3 | Breakout force | <0.5 gf | Must |
| R4 | Sensing | Contactless dual Hall, one tilting magnet | Must |
| R7 | Wired stimulus-to-report latency | <5 ms | Must |
| R8 | Effective resolution | >=8 bits/axis at 1 kHz, >=9 filtered | Must |
| R9 | Core BOM | <$35 wired, <$50 with battery | Must |
| R10 | Buildable with FDM printer + soldering iron | No machining, no reflow required | Must |
| R11 | Modes | Gamepad + mouse composite HID, runtime switch | Must |
| R11a | Layers | Rebind deflection at runtime from one shift input, no re-enumeration, no stuck buttons ([MODES.md](MODES.md)) | Must |
| R11b | Layer access | Shift source abstracted over Z-click / jacks / sip-puff; latching option; unused layers pruneable | Must |
| R12 | Configuration | USB CDC + web UI: deadzone, curve, tremor filter, profiles | Must |
| R13 | Persistence and updates | NVS profiles, OTA with A/B rollback | Should |
| R14 | Z press-to-click | <5 gf with clean hysteresis | Should |
| R15 | Battery life on BLE | 10+ hours | Should |

## 5. Success metrics

- **Physics:** R1/R3/R7/R8 met on the bench and published in [BENCH_LOG.md](BENCH_LOG.md) with raw data.
- **Durability:** flexure holds center within spec after a 30-day soak and 100k-cycle fatigue run (creep is the top risk).
- **Buildability:** 3 external builders complete a stick from the docs alone without maintainer intervention.
- **Users:** 5+ people with disabilities test the device and report usable control in real gameplay.

## 6. Key risks

| Risk | Mitigation |
|---|---|
| Printed flexure creeps or fatigues at sub-5 gf | Soak/fatigue test early; ball-in-PTFE + magnet centering kept as the fallback pivot |
| Hall noise floor misses >=8 effective bits | Bench dual TMAG5273 first; MLX90393 as the alternative |
| Gravity bias on a 3 gf pivot in non-desktop orientations | Weigh the moving assembly, re-derive the budget, consider firmware bias trim |
| Console compatibility overpromised | Publish an honest platform matrix; route consoles via XAC/adapter, not native protocols |
| Layer switching is too complex to learn, so users abandon it | Ship with two layers enabled and tap-to-swap only (phase L0); every other layer is opt-in |
| Prior-art IP contaminates an open repo | The layer system is a clean-room spec written from interaction concepts, credited to the J-Method authors; no third-party adapter code is read into, ported into, or LLM-translated into this repo ([MODES.md](MODES.md) section on provenance) |

## 7. Milestones

| Milestone | Exit criteria |
|---|---|
| M0 Bench proven | R1, R3, R8 measured on real hardware; pivot decision (flexure vs ball) locked |
| M1 Firmware spikes | USB composite HID + BLE HID streaming at 1 kHz; R7 measured |
| M2 Integrated prototype | Pod + main board + printed body working end to end as gamepad and mouse; layer phase L0 (tap-to-swap) proven |
| M3 Beta build | Assembly guide, BOM with links, configurator, 3 external builds; layer phase L1 (directional select, button layers) |
| M4 v1.0 | Success metrics met, tagged release, published to the accessibility maker community |

## 8. Open questions

1. Does the flexure survive creep, or does Stage A ship on the ball pod?
2. Is Z-click achievable on a flexure (axially stiff), or does it need a compliant mount or a discrete switch?
3. Single integrated board, or pod + main board split, given the multi-body goal?
4. Which single form factor ships first for v1.0: desktop base, chin boom, or thumb pad?
5. What is the default shift source before jacks and sip/puff exist (Stage B)? Z-click is the only candidate on a Stage A build, and it may fire during normal aiming (ties to question 2).
6. How many layers can a user actually hold in their head? The default set is eight, but the honest answer may be three.
