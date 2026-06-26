# Phase 0 Parts List

Everything needed to run the Phase 0 bench items in [TODO.md](../TODO.md). This is a
validation shopping list, not the production BOM ([HARDWARE.md](HARDWARE.md) has that).
One order, roughly **$100-150** total, less if you already own the tools.

Where exact SKUs are given they are high-confidence; otherwise the Search term column is the
reliable way to find the part. Prices are mid-2026 ballpark.

> **Mechanism note:** the magnets, washers, ball, and PTFE below build the **ball-in-PTFE bench
> rig**, which characterises the dual-Hall sensing independent of the pivot. To bench the
> **primary pivot**, also print the Tetra II flexure from
> [`models/tetra2-flexure/`](../models/tetra2-flexure/) (PLA/PETG, no extra purchases) and measure
> its force on the gram gauge.

---

## 1. Electronics Bench

| Item | Qty | Source / search term | Est. | Unblocks |
|------|-----|----------------------|------|----------|
| ESP32-S3-DevKitC-1 (N8R2 or N16R8) | 2 | Amazon "ESP32-S3-DevKitC-1 N16R8", or Espressif official store (cheaper, slower) | $10-16 ea | All firmware spikes; 2nd board = dongle/AS-Link peer. **Skip if you have spares from ATOS work** |
| TMAG5273 breakout, Qwiic | 2 | SparkFun "Qwiic TMAG5273" (standard or mini) | ~$12 ea | Sensing bench: the primary sensor |
| MLX90393 breakout | 1 | Adafruit #4022 "Wide-Range 3-Axis Magnetometer" | ~$15 | The fallback/upgrade comparison datapoint |
| TLV493D breakout | 1 (optional) | Adafruit #4366 | ~$6 | Cheap third opinion if TMAG results look odd |
| Qwiic/STEMMA-QT cables | 4 | "Qwiic cable 100mm" + one breadboard-pigtail variant | ~$6 | Wiring without soldering |
| Bare TMAG5273A1 chips | 0 now | Digi-Key/Mouser, ~$1-2 ea | | Defer to pod PCB order; breakouts are the bench path |

Bench note: two identical TMAG5273 breakouts share a default I2C address. The scaffold
firmware probes for the sensor on I2C0 and runs single-sensor; the dual-sensor cross-check
bench will put sensor B on the S3's second I2C controller, so no address surgery is needed.

## 2. Magnetics and Mechanics Bench

Buy a small size spread of magnets; tuning signal swing and centering force against gap is
exactly what Phase 0 is for. All NdFeB N52.

| Item | Qty | Source / search term | Est. | Unblocks |
|------|-----|----------------------|------|----------|
| Diametric discs 4x2 mm | 10 | K&J Magnetics (filter: diametrically magnetized) or AliExpress "4x2mm N52 diametric" | ~$8 | Sense magnet, nominal size |
| Diametric discs 3x2 mm and 6x3 mm | 5 + 5 | same | ~$10 | Signal-swing spread |
| Ring magnets ~10 OD x 5 ID x 2 mm | 5 | "ring magnet 10x5x2 N52" | ~$6 | Centering force |
| Ring/disc alternates 8x3x2 ring, 6x2 disc | 5 + 5 | same suppliers | ~$6 | Force-range spread for the adjuster sweep |
| Discs 3x1 mm | 20 | "3x1mm N52" (usually 50-packs) | ~$6 | Topper quick-swap |
| Steel washers M3 | pack | hardware store / "M3 flat washer" | ~$3 | Centering armature |
| Bearing balls 4 mm, chrome steel | 50-pack | "4mm bearing balls G25" | ~$7 | Pivot |
| Bearing balls 4 mm, Si3N4 ceramic | 10 (optional) | "4mm silicon nitride ball" | ~$10 | Lighter pivot if mass budget gets tight |
| PTFE rod 12 mm dia x 100 mm | 1 | McMaster "PTFE rod 1/2 inch" or AliExpress | ~$8 | Machined-cup reference path |
| PTFE thread seal tape | 1 roll | any hardware store | ~$1 | Budget printed-seat path |

## 3. Measurement Tools

| Item | Qty | Source / search term | Est. | Unblocks |
|------|-----|----------------------|------|----------|
| Digital pocket scale, 0.01 g resolution | 1 | "0.01g pocket scale 100g" | ~$14 | Force measurement (with the pulley rig below) and the moving-mass budget |
| Dial tension gauge 0-50 g | 1 (alternative) | "dial tension gauge 50g" (keyboard switch testers use these) | ~$25 | Direct full-throw force reading; nicer but optional if you have the scale |
| Feeler gauge set | 1 | "feeler gauge set metric" | ~$8 | Setting the 1.5 mm sensor gap (a 1.5 mm drill shank also works) |
| Calibration masses | free | a US nickel is 5.000 g; a quarter is 5.670 g | $0.30 | Sanity-check the scale/gauge |

Force rig: a printed pulley + thread lets the 0.01 g scale read lateral stick force as added
weight. The rig STL goes in `models/` once the pod v0 prints; until then, a smooth dowel as a
thread guide works.

## 4. Consumables and Hardware

| Item | Qty | Source / search term | Est. |
|------|-----|----------------------|------|
| Thin CA glue + accelerator | 1 | "thin CA glue kicker" | ~$10 |
| M2 + M3 brass/nylon screw assortment | 1 kit | "M2 M3 nylon screw assortment" | ~$8 |
| Carbon tube ~2.5 mm OD | 2-3 sticks | hobby/kite shop, "2.5mm carbon tube" | ~$6 |
| IPA 90%+ | 1 | pharmacy | ~$4 |
| Breadboard + jumpers | have? | any | ~$8 |

## 5. Defer to Stage B (do not order yet)

| Item | Why wait |
|------|----------|
| LiPo + BQ24074 + MAX17048 breakouts | Battery option lands after the pod works on USB |
| MPXV7002DP sip/puff sensor | Stage B feature |
| DRV2605L (Adafruit #2305) + LRA | Stage B feature |
| 3.5 mm jacks, USB-C parts, bare sensors | Arrive with the PCB order |

---

## Order Strategy

- **Amazon/SparkFun/Adafruit now, AliExpress in parallel:** breakouts and tools from fast
  sources so the sensing bench starts this week; the magnet size-spread from AliExpress is
  fine because every magnet gets characterized by the bench anyway (lot-to-lot spread is data,
  not a problem, and the TMAG5273 itself is the gauss meter once it runs).
- **First bench session needs only:** one devkit + one TMAG5273 breakout + one Qwiic cable +
  any diametric magnet. Everything else can trickle in.
- Keep magnets in labeled bags away from the scale (they pull on the load cell through the
  case and corrupt readings).

## Checklist Mapping

| TODO Phase 0 item | Needs from this list |
|-------------------|----------------------|
| Sensing bench (swing/noise/bits) | Devkit, TMAG5273 x1-2, Qwiic, diametric magnets |
| TMAG5273 vs MLX90393 decision | + MLX90393 breakout |
| Z-press detection | same as sensing bench |
| Centering force 1-8 gf | Ring magnets, washers, scale or gauge |
| Pivot breakout force | Balls, PTFE, scale or gauge |
| Pod v0 print + moving mass | Carbon tube, magnets, balls, 0.01 g scale |
| TinyUSB / BLE / latency spikes | Devkits only (firmware ships with a simulation mode) |
