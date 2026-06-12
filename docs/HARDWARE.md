# Hardware Documentation

Build reference for Alpha Stick **V2**. Rationale, trade studies, and the force/latency budgets
live in [DESIGN_V2.md](DESIGN_V2.md); this file is what you build. The V1 load-cell isometric
approach is archived in [ISOMETRIC_PARTS.md](ISOMETRIC_PARTS.md).

---

## Overview

Two boards, one cable:

- **Sensor pod (~20 x 20 mm):** the entire input mechanism. Ball pivot, tilting magnet,
  dual 3D Hall sensors, magnetic centering with a threaded force adjuster (1-8 gf).
- **Main board:** ESP32-S3, USB-C, 4x 3.5 mm switch jacks, options (battery, sip/puff,
  haptics), connected to the pod by a 6-pin cable.

The split matters: the pod drops into different bodies (desktop base, chin boom, mouth-stick
frame, thumb puck) without electronics changes.

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

### Mechanism

```
                topper (magnetic quick-swap, <1 g)
                  |
                  |   stick: carbon tube, 25/40/60 mm options
                  |
            +-----o-----+      pivot: 4 mm ball in PTFE cup
            |   ball    |      (magnet preload seats it, ~30 gf)
            +-----+-----+
                  |
          [steel washer]       centering armature, rigid with stick
             [N52 D4x2]        diametric sense magnet, tilts 1:1 with stick
                  |
         . . . . gap ~1.5 mm . . . .
       =============================    pod PCB
        [TMAG5273 A]   [TMAG5273 B]     dual 3D Hall, I2C
       =============================
              (ring magnet on            centering force + ball preload,
               threaded carrier)         turn to set 1-8 gf
```

How it works, in one paragraph: the magnet tilts with the stick, so the field *direction* at
the sensors rotates 1:1 with stick angle (direction, not magnitude, so temperature and gap
creep calibrate out). The ring magnet below attracts the steel washer on the stick: on-axis
that attraction is pure preload seating the ball; tilted, it is a restoring torque. Turning
the carrier changes the gap, which scales centering force and preload together.

### Pod parts

| Qty | Part | Spec | Notes | Est. |
|-----|------|------|-------|------|
| 2 | TMAG5273A1 | 3-axis Hall, I2C, +/-40/80 mT | Two different factory I2C address variants (trailing letter of the orderable part number sets the default address; pick two distinct ones per the TI datasheet) | $1.60 |
| 1 | Sense magnet | N52 disc, D4 x 2 mm, **diametric** | Diametric is load-bearing: axial will not work | $0.40 |
| 1 | Ring magnet | N52, ~D10/D5 x 2 mm, axial | In the adjuster carrier | $0.50 |
| 1 | Steel washer | M3, ~0.3 g | Centering armature on stick | $0.05 |
| 1 | Ball | 4 mm steel or ceramic, grade 25 | Pivot | $0.30 |
| 1 | PTFE cup | machined from 6 mm PTFE rod, or printed PCTG seat + PTFE tape | Budget path works, measure breakout | $0.50 |
| 1 | Stick tube | carbon, ~2.5 mm OD | Kite/hobby stock | $0.50 |
| 1 | Pod PCB | 2-layer, ~20 x 20 mm | | $1.00 |
| 1 | Carrier + housing | printed, M12 x 0.75 thread | Quarter-turn detents | print |
| 2 | Topper magnets | N52 D3 x 1 mm | One in stick, one per topper | $0.20 |

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

### Moving-mass budget (hard limit 2.5 g, see DESIGN_V2 section 3)

| Part | Mass |
|------|------|
| Carbon tube (40 mm) | ~0.4 g |
| Sense magnet D4x2 | ~0.19 g |
| Steel washer | ~0.3 g |
| Ball (4 mm steel) | ~0.26 g |
| Topper + magnet | <1.0 g |
| **Total** | **~2.1 g** |

Weigh every new topper design against this table. Ceramic ball saves ~0.15 g if needed.

### Assembly targets

- Magnet-to-sensor gap: 1.5 mm nominal (1.0-2.5 mm usable; calibration absorbs the build)
- Preload at center: ~30 gf (set by carrier position at the "3 gf" detent)
- Sensors symmetric about the pivot axis, ~3 mm apart
- No ferromagnetic fasteners in the pod except the intended washer; brass or nylon M2 hardware

### Force adjuster

Printed M12 x 0.75 external thread on the carrier, internal thread in the pod housing,
quarter-turn detent spring. Approximate mapping (characterize per build in Phase 0):

| Carrier position | Full-deflection force at 40 mm |
|---|---|
| Backed out (max gap) | ~1 gf, floaty |
| Nominal | 3 gf |
| Bottomed (min gap) | ~8 gf, near-isometric feel |

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
| Magnets | sense + ring + topper set | $2.50 |
| USB-C + ESD + LDO + passives | | $2.50 |
| Jacks | 4x PJ-320A | $1.60 |
| Buttons, LEDs, connectors | | $1.50 |
| PCBs | main + pod, JLCPCB 2-layer | $4.00 |
| Pivot hardware | ball, PTFE, brass/nylon M2-M3 | $2.00 |
| Printed parts | PLA/PETG, ~60 g | $3.00 |

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
5. Iterate the printed carrier thread until detents feel right
