# Alpha Stick: A Sub-5-Gram-Force, Open-Source Adaptive Joystick

**Owen Kent** | Alpha Stick Project | June 2026 | Draft 1.0

Firmware MIT, hardware CERN-OHL-P v2. Repository: github.com/owenpkent/alpha-stick

> **Mechanism update (June 2026):** the primary pivot is now the **Tetra II spherical flexure**
> — bearing-free, frictionless, and self-centering (see [DESIGN_V2.md](DESIGN_V2.md) section 5 and
> [`models/tetra2-flexure/`](../models/tetra2-flexure/)). The ball-in-PTFE pivot and the magnetic
> "force is a dial" adjuster described in the sections below are retained as the documented
> alternative/fallback; the contactless dual-Hall sensing, electronics, and force/latency targets
> are unchanged.

---

## Abstract

Standard game controller thumbsticks require 150 to 450 grams-force to
deflect, which excludes many people with neuromuscular conditions from both
gaming and, increasingly, from computer access in general. Devices that solve
the force problem exist but are expensive, proprietary, and single-purpose.
This paper presents Alpha Stick, an open-source joystick designed from first
principles to operate below 5 grams-force at roughly $25 in parts. Three
decisions follow from a force budget at the gram scale: position must be
sensed without contact (a tilting magnet read by Hall-effect sensors, using
field direction rather than magnitude for stability against temperature and
mechanical creep), the restoring force must be frictionless and adjustable
(magnetic attraction through a threaded carrier, 1 to 8 grams-force without
tools), and the only mechanical contact (the pivot) must contribute under 0.2
grams-force of friction. The same hardware enumerates as a USB/Bluetooth
gamepad, mouse, and keyboard, and a dual-sensor build with a TTL- and
sequence-guarded protocol allows it to serve as a drive input for the ATOS
open wheelchair platform under that system's safety authority. The design,
electronics, firmware architecture, and a falsifiable validation plan are
described; bench results are explicitly not yet claimed.

---

## 1. Introduction

The hardest part of any assistive technology setup is the input device. It is
the component that must match an individual body, the component that wears,
and the component whose commercial versions cost the most relative to what
they contain. For gaming specifically, the analog stick is the gateway: most
modern games are unplayable without smooth two-axis analog input.

The author is a wheelchair user with muscular dystrophy. The force floor of
commercial input devices is not an abstraction in this work; it is the
difference between playing and watching.

Alpha Stick's thesis is that the force problem is a physics problem, not a
product-tier problem. Solving it requires removing springs, potentiometers,
and sliding contacts from the force path entirely, and once that is done, the
remaining device is cheap. The project goal is a joystick that:

- deflects fully at under 5 grams-force, adjustable down to 1,
- costs about $25 in parts and is buildable with a 3D printer and a
  soldering iron,
- is one device for gaming (gamepad), desktop access (mouse and keyboard),
  and mobility (wheelchair drive input via a safety host),
- is fully open: hardware, firmware, and the reasoning behind both.

## 2. Background and prior art

Four existing systems define the landscape:

**QuadStick** is the reference mouth-operated controller: sip/puff plus a
low-force stick, multi-platform, and genuinely life-changing for its users.
It is also a single-vendor product at several hundred dollars.

**The Feather joystick (Celtic Magic)** demonstrated that sub-5-grams-force
operation with magnetic sensing is achievable and valuable, and is the
closest spiritual ancestor of this work. It remains a niche commercial
product rather than an open platform.

**The Xbox Adaptive Controller and PlayStation Access controller** solved
the hub problem: standardized 3.5 mm switch jacks and USB stick ports that
adaptive devices can plug into. They deliberately do not solve the analog
stick itself; both expect third-party sticks, which is the slot Alpha Stick
fills.

**Isometric (force-sensing) sticks**, from the TrackPoint to load-cell
designs, including this project's own v1 concept, eliminate motion entirely.
Section 4 explains why sustained-force input was rejected on fatigue grounds
in favor of small displacement at near-zero force.

The gap is a device that is simultaneously ultra-low-force, low-cost, open,
multi-role, and documented well enough to be built and modified by its users.

## 3. Requirements

| # | Requirement | Target |
|---|-------------|--------|
| R1 | Full-deflection force | <5 gf, nominal 3 gf, adjustable 1-8 gf at a 40 mm grip point |
| R2 | Throw | +/-4 to 6 degrees (+/-2.8 to 4.2 mm at 40 mm) |
| R3 | Breakout force (first motion) | <0.5 gf |
| R4 | Sensing | Contactless; no pots, no strain gauges in the force path |
| R5 | Wired latency, motion to USB report | <5 ms |
| R6 | Effective resolution | >=8 bits per axis at 1 kHz |
| R7 | Core bill of materials | <$35 |
| R8 | Build accessibility | FDM printer plus basic soldering; no machining |

## 4. The force budget: designing at the gram scale

At 3 grams-force full scale, effects that are negligible in a normal joystick
become the design. The budget at the 40 mm grip point:

| Source | Allocation | Mechanism |
|--------|-----------|-----------|
| Centering (the intended force) | 1.0-8.0 gf, nominal 3.0 | Magnetic attraction, adjustable gap |
| Pivot friction | <0.2 gf | PTFE-class contact, ~30 gf preload, 2 mm effective radius: friction torque is approximately mu * N * r = 0.075 * 30 gf * 2 mm = 4.5 gf-mm, or ~0.11 gf at the grip point |
| Seals and boots | 0 | Labyrinth gap; any contacting seal would exceed the friction budget on its own |
| Sensor | 0 | Contactless |
| Gravity bias | up to ~1.3 gf equivalent | Bounded by a moving-mass budget, corrected by a per-profile tare |

Two consequences are worth stating because they shaped everything else.

**Gravity is a first-order term.** A moving assembly of mass m with center of
gravity h above the pivot, mounted horizontally (a chin boom, a vertical
surface), experiences a constant torque m * g * h. At m = 2.5 g and
h = 20 mm this is 50 gf-mm, about 1.25 gf at the grip point, nearly half the
nominal centering force. The design therefore carries a hard moving-mass
budget of 2.5 g (carbon tube, hollow toppers under 1 g each, one small
magnet), a firmware mount-orientation tare stored per profile, and a
recommendation to run tilted mounts at a stiffer force setting.

**Sustained force is the wrong currency.** The project's v1 concept was an
isometric stick (four load cells, no motion), reasoning that zero motion
means zero range-of-motion demands. The flaw is physiological: an isometric
stick converts every held input into a continuously held force, which is
precisely what fatigues users with neuromuscular conditions. It also
sacrifices proprioceptive feedback and inherits strain-gauge drift, so the
center wanders with temperature. A small displacement (2 to 3 mm) at
near-zero force preserves the minimal range-of-motion benefit while letting
the user rest at a held position, and a stiff force setting approximates the
isometric feel for those who want it. The load-cell design is archived in
this repository with its parts list for anyone who wants to pursue it.

## 5. Sensing: measuring direction, not magnitude

A diametrically magnetized disc magnet (N52, 4 x 2 mm) is fixed to the
moving assembly and tilts 1:1 with the stick. One or two 3-axis Hall-effect
sensors (TI TMAG5273, I2C, roughly $0.80 each) sit 1.5 mm below it and
measure the field vector at 1 kHz with 8x averaging.

The estimator uses the **direction** of the field vector, not its magnitude,
mapped through a one-time calibration polynomial. This is the load-bearing
trick for a printed, glued, hobby-buildable device, because field direction
is first-order immune to:

- NdFeB temperature coefficient (about -0.12 %/degC affects magnitude, not
  direction),
- axial creep of printed parts (the gap changes magnitude, not direction),
- magnet strength tolerance between builds and suppliers.

All ferromagnetic parts move rigidly with the magnet, so their distortions
are constant in the stick frame and the calibration absorbs them.

Three further properties come free:

**A zero-force click.** Pressing the stick down compresses the pivot contact
and closes the magnet-sensor gap; the magnitude channel, useless for tilt, is
ideal for detecting this. The result is a press-to-click at the stick's own
sub-5-gram force with no switch in the force path, plus lift-off detection.

**Cheap redundancy.** A second TMAG5273 (a different factory I2C address
variant) reads the same magnet. The gaming build averages them; the drive
build (section 10) cross-checks them continuously and latches a fault on
disagreement. Drive-grade input plausibility for about $1.

**An honest noise story.** With a 40-100 mT bias field and +/-5 degrees of
rotation, the per-axis signal swing is +/-3.5 to 8 mT against sensor noise in
the tens of microtesla after averaging: an estimated 8 to 9 effective bits
per axis at 1 kHz, comparable to commercial thumbsticks, with the 16-bit
MLX90393 as a drop-in upgrade path if the bench disagrees. These are
estimates pending measurement (section 12).

## 6. Mechanism: constrained passive magnetics

Earnshaw's theorem (1842) rules out stable static levitation with permanent
magnets alone, so every "floating" joystick is actually a constraint plus a
force field. Alpha Stick embraces that split explicitly:

- **The constraint** is a single pivot: a hardened ball (or printed dome) in
  a PTFE-lined socket. A gimbal was rejected because four sliding contacts
  dominate feel at the gram scale; a flexure was rejected as the primary
  element because flexure stiffness is a spring whose printed-polymer creep
  moves the center over weeks.
- **The force field** is magnetic attraction between the moving assembly and
  a ring magnet (or steel armature) held in a **threaded carrier** beneath
  the sensor. On axis, the attraction is pure preload that seats the pivot;
  tilted, it is a restoring torque. The full-deflection torque required is
  small (3 gf at 40 mm is about 1.2 mN-m), well within reach of small N52
  parts at millimeter gaps.

Turning the carrier changes the working gap, which scales preload and
centering force together: a tool-free force dial from about 1 to 8
grams-force. For users whose strength varies day to day, this single
mechanical feature may matter more than any firmware option.

There are no springs, no potentiometers, no sliding electrical contacts, and
no parts that wear except a PTFE surface. The moving assembly is about 2.1 g
against the 2.5 g budget.

## 7. Electronics

The system is deliberately boring: an ESP32-S3 module (native USB OTG,
Bluetooth LE, WiFi), the Hall sensor pair on a 20 x 20 mm satellite pod PCB
connected by a 6-pin cable, four 3.5 mm jacks for the standard adaptive
switch ecosystem, USB-C with proper ESD protection, and optional blocks for
battery (power-path charger and fuel gauge), sip/puff pressure sensing, and a
haptic driver for non-visual confirmation. The pod-on-a-cable split matters:
the same input mechanism drops into a desktop base, a chin boom, a
mouth-stick frame, or a thumb puck without electronics changes.

Core BOM is approximately $22 (MCU module $3.50, both sensors $1.60, magnets
$2.50, the rest connectors, passives, PCBs, and printed parts). Battery,
sip/puff, and haptics add $13, $10, and $4 respectively.

## 8. Firmware

The firmware (ESP-IDF, FreeRTOS) is one composite input device with
switchable personalities. A 1 kHz sensing task reads both sensors, applies
the calibration map and processing pipeline, and publishes to a lock-free
latest-value mailbox; a router task maps the active mode onto USB HID
(gamepad, mouse, and keyboard report IDs in one never-re-enumerating
composite device), BLE HID (same reports), ESP-NOW (low-latency wireless to a
dongle), or the AS-Link wired protocol (section 10).

The pipeline order is: calibration map, mount-orientation tare, axis
rotation, radial deadzone (preserves diagonals), per-mode response curve,
adaptive smoothing, optional slew limiting, dithered 16-bit quantization.
Tremor handling uses the 1-euro filter (Casiez et al., CHI 2012), whose
cutoff rises with input speed so rest tremor is smoothed without adding lag
to deliberate motion, plus an optional 3-12 Hz notch for rhythmic intention
tremor. Wired motion-to-report latency budgets to 2-3 ms.

Configuration is a JSON profile system (deadzone, curves, filters, mappings,
mount tare) stored on-device, edited through a WiFi web UI or a WebSerial
channel, with profiles switchable from hardware. Updates are A/B OTA with
automatic rollback.

## 9. Accessibility as architecture

Several features that read as accessibility checkboxes are, in this design,
direct consequences of the architecture:

- **Dwell click**: in mouse mode the pointer resting within a small radius
  fires a click, so the device is fully usable with zero clicking force.
- **Z-press click**: the same sub-5-gram force as travel, from the magnitude
  channel, no switch.
- **Sip/puff as four buttons**: hard/soft sip and puff thresholds, the
  QuadStick convention, available in every mode.
- **The force dial**: strength varies day to day; the input device should
  too, without tools or settings menus.
- **Per-profile mount tare**: chin and vertical mounts are first-class, not
  an afterthought fighting gravity bias.
- **One stable USB identity**: mode switches never re-enumerate, because
  re-pairing and driver churn are themselves accessibility failures.

## 10. Wheelchair drive input and the ATOS integration

ATOS is an open, mixed-criticality platform for powered wheelchairs (part of
the RAMMP research program) whose Layer 1 is an ESP32-S3 safety node: a
non-swappable motor-control core plus capability-gated loadable modules that
may only *propose* motion ({linear, angular, TTL, sequence} structures the
host clamps and may ignore). ATOS's architecture documents design input-device
integration but no input device has been built for it.

Alpha Stick is designed to be that device, under a strict division of
authority: **the stick proposes, the ATOS host disposes.** The stick never
touches motors, never carries stop authority, and is treated by the host as
untrusted input subject to validation.

The integration is a 20-byte framed protocol (AS-Link) over UART or ESP-NOW
whose semantics deliberately mirror the ATOS proposal type: normalized Q15
axes, a TTL the receiver decays to zero on staleness, a monotonic sequence
number, health flags, and a CRC32, plus a 1 Hz health frame whose absence
marks the input dead regardless of data frames. On the ATOS side, a thin
gateway module (L3 command tier) maps frames to drive proposals through the
host's own curves and limits.

Driving is a different integrity domain from gaming, so it is a different
**build**, not a different setting: the drive build makes the dual-sensor
cross-check mandatory (disagreement beyond 3% of full scale for 50 ms latches
a zero-output fault recoverable only at neutral with explicit
acknowledgment), disables automatic re-centering entirely, holds output at
zero on boot until the stick is observed neutral, enables slew limiting by
default, and shuts down WiFi and BLE in wired mode. The design posture
follows ISO 7176-14 (power wheelchair control systems) as a reference
standard; no compliance is claimed, and the final safety envelope always
belongs to the ATOS host's limit governor.

## 11. Stage C: active magnetics (research direction)

Because the centering field is magnetic, it can eventually be electronic:
four planar coils in the pod PCB layers, driven closed-loop at 1 kHz from the
same Hall sensors, superimpose forces on the stick magnet. The required
full-scale torque (~1.2 mN-m) is small enough that this is plausible but not
yet demonstrated; the stage begins with a single go/no-go coil-force bench.

If it works, the payoff is a programmable force field no commercial adaptive
joystick offers: centering force set in firmware per profile (0 to ~10 gf),
virtual detents and walls, haptic confirmation ticks, and **active tremor
damping**, a velocity-proportional opposing force applied at the physical
layer rather than as filtering, with the passive behavior as the failure
fallback. Power (~100-300 mW active) makes this a wired or docked feature
first.

## 12. Validation methodology and current status

This project's credibility strategy is to publish targets before results.
As of June 2026: the design baseline, firmware scaffold, and a parametric
printable bench rig are complete; **no bench measurements have been taken
yet**. The Phase 0 plan measures, in order: Hall signal swing, noise, and
effective bits at 1 kHz; centering force versus carrier position (target
range 1-8 gf, gauge-verified); pivot breakout force (<0.5 gf) and
return-to-center repeatability; moving mass; and end-to-end wired latency
(<5 ms, logic-analyzer-verified). Stage A exit criteria additionally include
center drift under 1% of full scale over two hours, a 100k-cycle deflection
test, and the only metric that finally matters: daily use by the author for
gaming and desktop work.

Estimates in this paper that fail on the bench will be corrected in the
design documents with the measured values and reasoning, in public.

## 13. Cost and reproducibility

The entire device is reproducible from the repository: a parametric OpenSCAD
source generates the printable mechanism; the PCB design targets common
fabrication services; the firmware builds with the standard Espressif
toolchain; and the documentation set covers hardware, assembly, printing,
configuration, and the development workflow itself. Core cost is ~$22 plus
roughly 100 g of filament. No part requires machining, and the only
specialized assembly skill is gluing small magnets in the correct
orientation, which the assembly guide treats with appropriate paranoia.

## 14. Limitations and open problems

Honest unknowns, with their planned resolutions:

1. **Sensor SNR** may fall below 8 effective bits at +/-5 degrees (first
   bench item; MLX90393 fallback footprint exists).
2. **PTFE stiction** may be perceptible below 1 gf (breakout measured
   explicitly; jewel-bearing fallback identified).
3. **Magnetic centering force range** at the available gaps is estimated,
   not yet gauge-verified (the bench rig's entire purpose).
4. **Printed-thread quality** for the force adjuster varies by printer
   (documented tuning parameter; metal-insert fallback).
5. **Platform compatibility** beyond PC (XAC USB stick ports, Switch BLE) is
   designed-for but unverified (early Phase 0 test).
6. **ATOS-side host service** for external input streams does not exist yet
   in the ATOS ABI (proposed as a capability addition; the gateway can be
   prototyped against a host stub).
7. **Stage C coil force** is plausible on paper only.

## 15. Roadmap

Stage A "Glide": the core stick, USB and BLE, web configuration, bench-passed
exit criteria. Stage B "Bridge": the ecosystem release: switch jacks,
sip/puff, chin/mouth/thumb bodies, the ESP-NOW dongle, AS-Link and the ATOS
gateway prototype, platform verification, kits. Stage C "Field": the active
coil layer, beginning with its feasibility bench.

## 16. Conclusion

The adaptive input problem is usually attacked as a product problem, which
produces excellent $400 devices. Treating it as a physics problem (remove
every contact and every spring from the force path, then measure direction
instead of magnitude so cheap materials stay stable) produces a $25 device
that is also a better architecture: the same contactless core scales from a
gaming stick to a mouse to a redundant, protocol-guarded wheelchair input.
Everything here is open, and everything here is falsifiable on a workbench,
which is where it goes next.

## Acknowledgments

The disability gaming community, AbleGamers, SpecialEffect, and Makers Making
Change for advocacy and groundwork; the QuadStick and Feather projects for
proving the categories; the ATOS/RAMMP program for the safety architecture
this design integrates with.

## References

- QuadStick: quadstick.com
- Feather joystick, Celtic Magic: celticmagic.org/feather
- Xbox Adaptive Controller: xbox.com/accessories/controllers/xbox-adaptive-controller
- PlayStation Access controller: playstation.com/accessories/access-controller
- ATOS repository and Layer 1 whitepaper (module ABI, drive proposals)
- S. Earnshaw, "On the nature of the molecular forces..." Trans. Camb. Phil. Soc., 1842 (Earnshaw's theorem)
- G. Casiez, N. Roussel, D. Vogel, "1 € Filter: A Simple Speed-based Low-pass Filter for Noisy Input in Interactive Systems," CHI 2012
- ISO 7176-14, Wheelchairs: Power and control systems for electrically powered wheelchairs and scooters
- TI TMAG5273 datasheet; Melexis MLX90393 datasheet
- This repository: DESIGN_V2.md (working baseline), HARDWARE.md, FIRMWARE.md, BENCH_LOG.md
