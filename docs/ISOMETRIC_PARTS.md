# Isometric Joystick Parts List

> **Archived (V2):** the load-cell isometric approach was dropped in the V2 redesign; see
> [DESIGN_V2.md](DESIGN_V2.md) section 2 for the rationale (sustained-force fatigue, strain
> gauge drift, complexity). Kept for reference and for anyone who wants to build it anyway.

Specific parts and links for building an isometric (force-sensing, no-movement) joystick for Alpha Stick.

---

## Quick Summary

| Component | Budget Option | Premium Option |
|-----------|--------------|----------------|
| Load Cells (x4) | 50g micro load cells | 100g TAL221 (SparkFun) |
| ADC (x2) | HX711 modules | NAU7802 (Adafruit/SparkFun) |
| I2C Mux (if NAU7802) | PCA9548 module | Adafruit PCA9548 |
| **Total Cost** | **~$10-15** | **~$35-45** |

---

## Load Cells

### Budget: Generic 50g/100g Micro Load Cells

**AliExpress (~$1-2 each):**
- Search: "50g micro load cell" or "100g micro load cell strain gauge"
- Look for: 4-wire Wheatstone bridge, rated output ~0.6-1.0 mV/V
- Typical listings: ~$0.50-2 per cell

**Amazon (~$3-5 each):**
- [100g Micro Load Cell](https://www.amazon.com/dp/B0C9LDHFPK) — ~$8 for sensor + HX711
- [100g Load Sensor](https://www.amazon.com/dp/B07WSBC3W4) — ~$7
- Search "100g load cell arduino" for more options

### Premium: SparkFun TAL221 Mini Load Cell

**SparkFun / Amazon (~$5-7 each):**
- [SparkFun Mini Load Cell - 100g (TAL221)](https://www.amazon.com/dp/B07G5KVXT5) — ~$7
- Well-documented, consistent quality
- Rated capacity: 100g
- Output: 0.7 ± 0.15 mV/V

---

## ADC Modules

### Budget: HX711 (24-bit, ~$1-3 each)

**Amazon:**
- [DIYmall 2pcs HX711 Module](https://www.amazon.com/dp/B010FG9RXO) — ~$7 for 2
- [WWZMDiB 4pcs HX711](https://www.amazon.com/dp/B0BLND4VF6) — ~$9 for 4
- [5pcs HX711 + Jumper Wires Bundle](https://www.amazon.com/dp/B0CGZLQB8W) — ~$10

**AliExpress:**
- Search: "HX711 module" — ~$0.50-1 each

**Specs:**
- 24-bit resolution
- 10 or 80 SPS sample rate
- 2-wire interface (DT, SCK)
- Requires 2 modules for X and Y axes

### Premium: NAU7802 (24-bit, I2C, ~$7-10 each)

**Adafruit:**
- [Adafruit NAU7802 24-Bit ADC](https://www.adafruit.com/product/4538) — $6.95

**Amazon:**
- [Adafruit NAU7802 STEMMA QT](https://www.amazon.com/dp/B0CTGR5499) — ~$10
- [SparkFun Qwiic Scale NAU7802](https://www.amazon.com/dp/B082MPLRCS) — ~$16

**Specs:**
- 24-bit resolution
- Up to 320 SPS sample rate
- I2C interface (fixed address 0x2A)
- Lower noise than HX711
- Requires I2C multiplexer for 2 units

---

## I2C Multiplexer (Required for 2x NAU7802)

**Adafruit:**
- [Adafruit PCA9548 8-Channel I2C Mux](https://www.adafruit.com/product/5626) — $5.95

**Amazon:**
- [PCA9548A I2C Mux Module](https://www.amazon.com/s?k=PCA9548+I2C+multiplexer) — ~$5-8

**AliExpress:**
- Search: "PCA9548 module" — ~$1-2

---

## Recommended Kits

### Budget Build (~$12-18)

| Qty | Part | Source | Price |
|-----|------|--------|-------|
| 4 | 100g Micro Load Cell | AliExpress | $4 |
| 2 | HX711 Module | AliExpress | $2 |
| 1 | Jumper wires | — | $2 |
| | | **Total** | **~$8-12** |

Or from Amazon:
| Qty | Part | Source | Price |
|-----|------|--------|-------|
| 4 | 100g Load Cell + HX711 kit | Amazon | $15-20 |

### Premium Build (~$40-50)

| Qty | Part | Source | Price |
|-----|------|--------|-------|
| 4 | SparkFun TAL221 100g | SparkFun/Amazon | $28 |
| 2 | Adafruit NAU7802 | Adafruit | $14 |
| 1 | PCA9548 I2C Mux | Adafruit | $6 |
| 1 | STEMMA QT cables | Adafruit | $3 |
| | | **Total** | **~$51** |

---

## Wiring Quick Reference

### HX711 Setup (Budget)

```
ESP32-S3          HX711 #1 (X-axis)     Load Cells X+/X-
─────────         ────────────────      ────────────────
GPIO4 ──────────▶ DT                    
GPIO5 ──────────▶ SCK                   E+/E- ◀── power
3.3V  ──────────▶ VCC                   A+/A- ◀── signal
GND   ──────────▶ GND                   

ESP32-S3          HX711 #2 (Y-axis)     Load Cells Y+/Y-
─────────         ────────────────      ────────────────
GPIO6 ──────────▶ DT
GPIO7 ──────────▶ SCK
3.3V  ──────────▶ VCC
GND   ──────────▶ GND
```

### NAU7802 Setup (Premium)

```
ESP32-S3          PCA9548 Mux           NAU7802 #1 & #2
─────────         ───────────           ───────────────
GPIO8 (SDA) ────▶ SDA ─────────────────▶ SDA (both)
GPIO9 (SCL) ────▶ SCL ─────────────────▶ SCL (both)
3.3V  ──────────▶ VCC                   VCC
GND   ──────────▶ GND                   GND
                  CH0 ─────────────────▶ NAU7802 #1
                  CH1 ─────────────────▶ NAU7802 #2
```

---

## Software Libraries

### For HX711:
- **Arduino:** [bogde/HX711](https://github.com/bogde/HX711)
- Install via PlatformIO: `lib_deps = bogde/HX711`

### For NAU7802:
- **Arduino:** [Adafruit_NAU7802](https://github.com/adafruit/Adafruit_NAU7802)
- **SparkFun:** [SparkFun_Qwiic_Scale_NAU7802](https://github.com/sparkfun/SparkFun_Qwiic_Scale_NAU7802_Arduino_Library)
- Install via PlatformIO: `lib_deps = adafruit/Adafruit NAU7802 Library`

### For PCA9548 I2C Mux:
- **Adafruit:** [Adafruit_TCA9548A](https://github.com/adafruit/Adafruit_TCA9548A)
- Install via PlatformIO: `lib_deps = adafruit/Adafruit TCA9548A Library`

---

## Notes

- **50g vs 100g:** 50g cells are more sensitive but easier to overload. 100g recommended for durability.
- **Preload:** Load cells work best with slight preload. Design mounting to compress cells ~1-2g at rest.
- **Shielding:** Keep wires short and away from power lines to reduce noise.
- **Calibration:** Will need per-unit calibration in firmware. Store calibration values in ESP32 NVS.

---

## Next Steps

1. Order parts (budget or premium kit)
2. Design 3D printed mounting bracket
3. Wire up prototype on breadboard
4. Test sensitivity and noise levels
5. Iterate on mechanical design

See [HARDWARE.md](HARDWARE.md) for full isometric joystick documentation.
