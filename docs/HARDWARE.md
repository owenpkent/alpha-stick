# Hardware Documentation

Build reference for Alpha Stick **V2**. Rationale, trade studies, and the force/latency budgets
live in [DESIGN_V2.md](DESIGN_V2.md); this file is what you build. The V1 load-cell isometric
approach is archived in [ISOMETRIC_PARTS.md](ISOMETRIC_PARTS.md).

---

## Overview

Two boards, one cable:

- **Sensor pod (~20 x 20 mm PCB + flexure joint):** the entire input mechanism. The stick
  pivots on a **Tetra II spherical flexure** (compliant, bearing-free, self-centering — no
  pivot ball, no centering magnet, no force adjuster); a tilting diametric magnet on the
  flexure's moving platform is read by dual 3D Hall sensors.
- **Main board:** ESP32-S3, USB-C, 4x 3.5 mm switch jacks, options (battery, sip/puff,
  haptics), connected to the pod by a 6-pin cable.

The split matters: the pod drops into different bodies (desktop base, chin boom, mouth-stick
frame, thumb puck) without electronics changes.

The **ball-in-PTFE-cup pod with magnetic centering** is retained as the
[alternative mechanism](#alternative-mechanism-ball-in-ptfe-pod) (the original V2 path) and as
the bench rig that characterises the sensing independent of the pivot choice.

```
                      +---------------------------------------+
   sensor pod         |              main board               |
  +--------------+    |                                       |
  | TMAG5273 x2  |-6p-|  ESP32-S3-MINI-1                      |
  | magnet, ball |    |   |- USB OTG ---- USB-C --> PC / XAC  |
  | force adjust |    |   |- BLE HID -------------> wireless  |
  +--------------+    |   |- ESP-NOW -----> dongle / ATOS S3  |
                      |   |- UART AS-Link -------> ATOS S3    |
  4x 3.5mm jacks -----|   |- WiFi AP -----> web config + OTA  |
  2x tactile ---------|                                       |
  sip/puff (opt) -----|  BQ24074 + MAX17048 (battery option)  |
                      |  WS2812 x2   DRV2605L + LRA (option)  |
                      +---------------------------------------+
```

---

## Sensor Pod

![Tetra II flexure joint — isometric, front, side](../models/tetra2-flexure/preview-iso.png)

### Mechanism

```
                topper (magnetic quick-swap, <1 g)
                  |
                  |   stick: carbon tube, 25/40/60 mm options
                  |
              [ P ]            remote rotation centre, ~50 mm out (set by flexure)
                  :
        ##########:##########   Tetra II spherical flexure (printed, one piece):
        ##  nested blade   ##   nested tetrahedron blade flexures, 3-DOF rotation
        ##  flexures       ##   about P, no sliding surfaces, elastic return to centre
        #####moving########      moving platform tilts 1:1 with the stick
            [N52 D4x2]           diametric sense magnet, rigid with the platform
                  |
         . . . . gap ~1.5 mm . . . .
       =============================    pod PCB
        [TMAG5273 A]   [TMAG5273 B]     dual 3D Hall, I2C
       =============================    fixed (flexure base mounts to it)
```

How it works, in one paragraph: the stick mounts to the flexure's moving platform and pivots
about the flexure's remote centre **P**. The blade flexures bend elastically, so they *are* the
pivot and the return-to-centre spring at once — there is no ball, no PTFE cup, no centering
magnet, and no force adjuster, and because nothing slides there is no break-away friction (the
first gram of input already moves it). The sense magnet rides the moving platform, so the field
*direction* at the sensors rotates 1:1 with stick angle (direction, not magnitude, so
temperature and gap creep calibrate out) — sensing is unchanged from the ball-pivot pod.

**Force is set by the flexure, not a dial.** Full-deflection force is fixed by blade geometry,
material, and scale, not adjusted with a thread. The paper's closed-form stiffness matches FEA
only qualitatively for the real tetrahedron element (NMAE ~35%), so treat any calculated figure
as a starting point and **measure force on the bench** (Phase 0). To retune, swap in a flexure
with different blade thickness or a scaled STEP (re-measure after any scale change — scaling
moves P and stiffens the blades non-linearly). Programmable force returns later via the Stage C
active-coil path (DESIGN_V2 section 5), not via the mechanism here.

### Pod parts

| Qty | Part | Spec | Notes | Est. |
|-----|------|------|-------|------|
| 2 | TMAG5273A1 | 3-axis Hall, I2C, +/-40/80 mT | Two different factory I2C address variants (trailing letter of the orderable part number sets the default address; pick two distinct ones per the TI datasheet) | $1.60 |
| 1 | Sense magnet | N52 disc, D4 x 2 mm, **diametric** | On the flexure's moving platform. Diametric is load-bearing: axial will not work | $0.40 |
| 1 | Tetra II flexure | printed PLA/PETG, single piece, ~0.7 mm blade walls | The pivot **and** the centering spring; see [`models/tetra2-flexure/`](../models/tetra2-flexure/) (CC-BY) | print |
| 1 | Stick tube | carbon, ~2.5 mm OD | Kite/hobby stock; mounts to the flexure platform so its natural pivot sits at P | $0.50 |
| 1 | Pod PCB | 2-layer, ~20 x 20 mm | Flexure base mounts to it | $1.00 |
| 1 | Flexure mount / pod housing | printed | Locates flexure base over the PCB, sets the 1.5 mm magnet gap | print |
| 2 | Topper magnets | N52 D3 x 1 mm | One in stick, one per topper | $0.20 |

Dropped from the ball-pivot pod: the 4 mm ball, PTFE cup, ring centering magnet, steel
armature washer, and the M12 x 0.75 threaded force-adjuster carrier — the flexure replaces all
of them. They live on in the [alternative mechanism](#alternative-mechanism-ball-in-ptfe-pod).

### Pod connector (JST-SH 1.0 mm, 6-pin)

| Pin | Signal |
|-----|--------|
| 1 | 3V3 |
| 2 | GND |
| 3 | SDA |
| 4 | SCL |
| 5 | INT_A (sensor A conversion-ready) |
| 6 | INT_B (sensor B conversion-ready) |

I2C at 400 kHz minimum, 1 MHz target. Keep the cable under ~30 cm for 1 MHz; chin-boom builds
with longer runs drop to 400 kHz (still comfortably supports 1 kHz sampling).

### Moving-mass budget (gravity bias, see DESIGN_V2 section 3)

The ball-pivot pod held the moving stick assembly under 2.5 g so a tilted-mount gravity bias
stayed below the centering force. The flexure changes the accounting: it has no ball or steel
washer, but its **moving platform and inner blades** add distributed mass that the old table did
not, and there is no adjustable centering to fight gravity with — the blades' fixed restoring
torque does. So re-derive the budget against the *measured* flexure stiffness in Phase 0.

| Part | Mass |
|------|------|
| Carbon tube (40 mm) | ~0.4 g |
| Sense magnet D4x2 | ~0.19 g |
| Topper + magnet | <1.0 g |
| Flexure moving platform + inner blades | measure (was 0 in ball-pivot accounting) |
| **Target** | keep gravity torque well under the flexure's restoring torque |

Weigh every new topper against this, and prefer stiffer-blade flexures for chin/vertical mounts
so the stick still returns to centre against gravity.

### Assembly targets

- Magnet-to-sensor gap: 1.5 mm nominal (1.0-2.5 mm usable; calibration absorbs the build)
- Stick's natural pivot aligned with the flexure's remote centre **P** (~50 mm out); the flexure
  base seated flat and square to the PCB so the moving platform tilts about P, not askew
- Sensors symmetric about the pivot axis, ~3 mm apart
- No ferromagnetic fasteners in the pod; brass or nylon M2 hardware
- Inspect blades after printing — a cracked or under-extruded blade changes stiffness and is a
  single-point failure (a blade break scraps the one-piece joint)

---

## Alternative mechanism: ball-in-PTFE pod

The original V2 pivot, retained as the fallback if printed-flexure creep/fatigue proves
unacceptable on the bench, and as the rig that characterises the sensing independent of the
pivot. The sensing, connector, and main board are identical to the flexure pod above.

```
            +-----o-----+      pivot: 4 mm ball in PTFE cup
            |   ball    |      (magnet preload seats it, ~30 gf)
            +-----+-----+
                  |
          [steel washer]       centering armature, rigid with stick
             [N52 D4x2]        diametric sense magnet, tilts 1:1 with stick
                  |
         . . . . gap ~1.5 mm . . . .
       =============================    pod PCB, dual TMAG5273
              (ring magnet on            centering force + ball preload,
               threaded carrier)         turn to set 1-8 gf
```

A ring magnet in a threaded carrier attracts a steel washer on the stick: on-axis the
attraction is pure preload seating the ball; tilted, it is a restoring torque. Turning the
carrier changes the gap, scaling centering force and preload together.

**Extra parts vs the flexure pod:** 4 mm steel/ceramic ball (grade 25, ~$0.30), PTFE cup
(machined 6 mm rod or printed PCTG seat + PTFE tape, ~$0.50), N52 ~D10/D5 x 2 mm axial ring
magnet (~$0.50), M3 steel armature washer (~$0.05), printed M12 x 0.75 carrier + housing with
quarter-turn detents.

**Force adjuster mapping** (characterize per build in Phase 0):

| Carrier position | Full-deflection force at 40 mm |
|---|---|
| Backed out (max gap) | ~1 gf, floaty |
| Nominal | 3 gf |
| Bottomed (min gap) | ~8 gf, near-isometric feel |

Moving-mass budget for this variant is the original <2.5 g (carbon tube ~0.4 g + sense magnet
~0.19 g + steel washer ~0.3 g + 4 mm ball ~0.26 g + topper <1.0 g ≈ 2.1 g; ceramic ball saves
~0.15 g). The build steps live in [ASSEMBLY.md](ASSEMBLY.md) and the printable parts in
[`models/`](../models/README.md) (Pod v0).

---

## Main Board

### Core

| Block | Part | Notes |
|---|---|---|
| MCU | ESP32-S3-MINI-1-N8 | Native USB OTG, BLE 5, WiFi; same target as ATOS |
| USB | USB-C 16-pin (TYPE-C-31-M-12 class) | 5.1k CC pulldowns, device role |
| USB ESD | USBLC6-2SC6 | On D+/D- |
| Regulator | RT9080-33 (or ME6217C33) | 3V3, low IQ |
| Jacks | 4x PJ-320A (TRRS footprint) | Wired tip+sleeve as mono switch inputs; ring routed to a test pad, reserved for future TRS analog accessories |
| Buttons | 2x 6 x 6 mm tactile | Mode/pair/tare combos |
| Status LEDs | 2x WS2812B-2020 | Keep >=15 mm from the pod |
| Expansion | 1x Qwiic/STEMMA-QT (JST-SH 4-pin) | Shares I2C0 |

### Options (fit the footprints, populate as needed)

| Block | Part | Notes |
|---|---|---|
| Charger | BQ24074 | Power-path: runs while charging, clean USB/battery switchover |
| Fuel gauge | MAX17048 | I2C, real state-of-charge |
| Battery | LiPo 503035, 500 mAh, JST-PH | Protected cell |
| Power switch | SS12D00 slide | In battery path |
| Sip/puff | MPXV7002DP (+/-2 kPa differential, analog) | Or Honeywell ABP I2C variant on the Qwiic port |
| Haptics | DRV2605L + 8 mm LRA | Mount the LRA in the base mass, isolation grommet, far side from the pod: it contains a magnet and it shakes |

### Pin assignments (draft, final at schematic review)

| Function | GPIO | Type | Notes |
|----------|------|------|-------|
| I2C0 SDA | 8 | I2C | Pod, fuel gauge, haptic, Qwiic; 2.2k pullups |
| I2C0 SCL | 9 | I2C | |
| Pod INT_A | 10 | Input | Conversion-ready, sensor A |
| Pod INT_B | 11 | Input | Conversion-ready, sensor B |
| Jack 1-4 | 4 / 5 / 6 / 7 | Input | 10k pullup + 100 nF + ESD |
| Button 1-2 | 12 / 13 | Input | Internal pullup |
| Haptic EN | 14 | Output | DRV2605L enable |
| Sip/puff | 1 | ADC1_CH0 | Analog option, RC filtered |
| AS-Link TX / RX | 17 / 18 | UART1 | To ATOS host, 1 Mbaud |
| WS2812 | 48 | Output | |
| USB D- / D+ | 19 / 20 | USB | Fixed function |

Strapping pins 0, 3, 45, 46 are left unconnected or output-only.

### 3.5 mm jack wiring (XAC-style switch inputs)

```
        +-----+
  Tip --|     |-- GPIO (10k pullup to 3V3, 100 nF to GND, ESD diode)
 Ring --|     |-- test pad (reserved)
Sleeve--|     |-- GND
        +-----+

Switch closed: GPIO LOW.  Debounce in firmware, 10-100 ms configurable.
```

---

## Power

| Mode | Source | Est. draw | Runtime |
|---|---|---|---|
| USB wired | 5 V USB-C | 80-120 mA typ (WiFi off) | n/a |
| BLE on battery | 500 mAh LiPo | 30-45 mA avg | ~11-16 h (validate in Phase 0) |
| ESP-NOW on battery | 500 mAh LiPo | similar to BLE | similar |
| Config mode (WiFi AP) | either | up to 500 mA peaks | keep on USB |

Battery path: USB-C -> BQ24074 (power path + charge) -> 3V3 LDO; MAX17048 on I2C for
state-of-charge; slide switch between cell and system. USB-only builds omit all three parts.

---

## Bill of Materials

### Core (wired build, ~$22)

| Item | Part | Est. |
|------|------|------|
| MCU module | ESP32-S3-MINI-1-N8 | $3.50 |
| Hall sensors | 2x TMAG5273A1 (distinct address variants) | $1.60 |
| Magnets | sense + topper set (no ring centering magnet) | $2.00 |
| USB-C + ESD + LDO + passives | | $2.50 |
| Jacks | 4x PJ-320A | $1.60 |
| Buttons, LEDs, connectors | | $1.50 |
| PCBs | main + pod, JLCPCB 2-layer | $4.00 |
| Pivot hardware | brass/nylon M2-M3 (flexure is printed) | $0.50 |
| Printed parts | PLA/PETG incl. Tetra II flexure, ~70 g | $3.50 |

The ball-in-PTFE pod adds ball + PTFE + ring magnet + carrier (~$3.35); see the
[alternative mechanism](#alternative-mechanism-ball-in-ptfe-pod).

### Options

| Option | Parts | Est. |
|--------|-------|------|
| Battery | LiPo 503035 + BQ24074 + MAX17048 + switch | +$13 |
| Sip/puff | MPXV7002DP + tubing + mouthpiece | +$10 |
| Haptics | DRV2605L + LRA | +$4 |
| ESP-NOW dongle | minimal second S3 board | +$9 |

---

## PCB Guidelines

| Parameter | Main board | Pod |
|-----------|------------|-----|
| Layers | 2 | 2 |
| Thickness | 1.6 mm | 1.0 mm (fits the gap stack) |
| Copper | 1 oz | 1 oz |
| Finish | HASL ok | ENIG preferred (flat under sensors) |

1. **Pod:** solid ground pour, sensors on top, connector on bottom edge, no switching
   regulators or LEDs on the pod, panelize 4-up.
2. **Magnetic keep-out:** WS2812s, the LRA, and any speaker >=15 mm from the pod. Static
   fields calibrate out; switched currents and moving magnets do not.
3. I2C pullups 2.2k for 1 MHz operation.
4. 100 nF decoupling at every IC, bulk 10 uF at the module.
5. USB D+/D- as a 90-ohm differential pair, short.
6. Test points: 3V3, GND, SDA, SCL, AS-Link TX/RX.

---

## Safety Notes

- **Small magnets** are an ingestion hazard; glue all press-fit magnets (cyanoacrylate) and
  note them in the assembly guide.
- NdFeB is brittle: press by hand pressure only, never hammer.
- People with **pacemakers/implants** should follow their device manufacturer's magnet
  distance guidance; the fields here are small but real.
- LiPo: protected cells only, no charging unattended, observe polarity at the JST-PH.
- Mount securely (AMPS pattern / 1/4-20 insert); a falling unit near medical equipment is the
  failure mode that matters.

---

## Sourcing

| Part class | Source |
|------------|--------|
| ICs, passives, connectors | LCSC (JLC assembly compatible), Digi-Key for one-offs |
| Magnets | K&J Magnetics (spec-true), AliExpress (verify with a gauss meter or accept lot-to-lot spread; calibration absorbs most of it) |
| Ball, PTFE rod | McMaster-Carr, or bearing suppliers on AliExpress |
| Carbon tube | Hobby/kite shops |
| PCBs | JLCPCB |

---

## Validation

Every number above with "validate" or "characterize" next to it is a checkbox in
[TODO.md](../TODO.md) Phase 0. The pod's exit criteria (force, breakout, noise, latency,
drift, cycle life) are in [DESIGN_V2.md](DESIGN_V2.md) section 10.

## Next Steps

1. Phase 0 bench items (TODO.md)
2. Pod schematic + layout, panelized
3. Main board schematic + layout
4. Order boards + magnets together; calibration sweep on first articles
5. Print the Tetra II flexure, measure full-deflection force on the gram gauge, and iterate
   blade thickness / scale until the nominal feel target is met (bench, not calculated)
