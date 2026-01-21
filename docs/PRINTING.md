# 3D Printing Guide

Instructions for printing Alpha Stick enclosure and accessories.

---

## Overview

Alpha Stick uses a modular 3D-printed enclosure designed for:

- **Easy printing** — No supports needed for most parts
- **Easy assembly** — Snap-fit and screw connections
- **Customization** — Multiple top plate options
- **Mounting** — Compatible with standard camera mounts

---

## Print Files Location

```
models/
├── stl/                    # Ready-to-print files
│   ├── base-v1.stl         # Main enclosure base
│   ├── top-standard-v1.stl # Standard top plate
│   ├── top-palm-v1.stl     # Palm rest variant
│   ├── mount-adapter.stl   # 1/4-20 camera mount
│   ├── button-cap-flat.stl # Flat button caps
│   ├── button-cap-dome.stl # Domed button caps
│   └── joystick-knob.stl   # Custom joystick topper
├── step/                   # STEP files for modification
└── source/                 # FreeCAD/Fusion360 files
```

---

## Recommended Print Settings

### General Settings

| Setting | Value | Notes |
|---------|-------|-------|
| Layer Height | 0.2mm | 0.16mm for finer detail |
| Wall Count | 3 | Structural strength |
| Infill | 20% | 30% for durability |
| Top/Bottom Layers | 4 | Good surface finish |
| Print Speed | 50mm/s | Slower for better quality |
| Supports | None | Designed to print without |
| Adhesion | Brim | Optional, helps prevent warping |

### Per-Part Settings

| Part | Layer Height | Infill | Notes |
|------|--------------|--------|-------|
| Base | 0.2mm | 20% | Standard settings |
| Top plate | 0.2mm | 20% | Flip upside-down to print |
| Mount adapter | 0.16mm | 40% | Higher infill for thread strength |
| Button caps | 0.12mm | 100% | Solid for durability |
| Joystick knob | 0.12mm | 100% | Solid, smooth finish |

---

## Material Recommendations

### PLA (Recommended for beginners)

- **Pros:** Easy to print, low warping, no heated enclosure needed
- **Cons:** Can warp in hot cars, less impact resistant
- **Temp:** 200-210°C nozzle, 60°C bed
- **Best for:** Indoor use, prototyping

### PETG (Recommended for durability)

- **Pros:** More heat resistant, better impact resistance, flexible
- **Cons:** Stringing, needs higher temps
- **Temp:** 230-250°C nozzle, 80°C bed
- **Best for:** Final builds, travel use

### TPU (For soft-touch parts)

- **Pros:** Soft, grippy, impact absorbing
- **Cons:** Difficult to print, requires direct drive extruder
- **Temp:** 220-240°C nozzle, 40°C bed
- **Best for:** Joystick knobs, palm rests, button caps

### ABS (Not recommended)

- **Cons:** Warping, fumes, requires enclosure
- **Use only if:** You need high heat resistance (>80°C)

---

## Printer Requirements

### Minimum Build Volume

| Part | Size (mm) | Notes |
|------|-----------|-------|
| Base | 120 x 80 x 40 | Fits most printers |
| Top plate | 120 x 80 x 15 | Same footprint as base |
| Mount adapter | 40 x 40 x 20 | Small part |

**Minimum printer:** 150 x 150 x 100mm build volume (Ender 3, Prusa Mini, etc.)

### Recommended Printers

| Printer | Price | Notes |
|---------|-------|-------|
| Creality Ender 3 V2 | $250 | Great value, large community |
| Prusa Mini+ | $400 | Reliable, good quality |
| Bambu Lab A1 Mini | $300 | Fast, easy to use |
| Anycubic Kobra 2 | $200 | Budget option |

---

## Print Orientation

### Base

```
Print flat, bottom-down:

    ┌─────────────────┐
    │                 │  ← Top (open)
    │                 │
    │                 │
    └─────────────────┘  ← Bottom (on bed)
```

### Top Plate

```
Print upside-down for smooth top surface:

    ┌─────────────────┐  ← Bottom (on bed)
    │                 │
    └─────────────────┘  ← Top (smooth)
```

### Mount Adapter

```
Print thread-side up:

         ┌───┐
         │   │  ← Thread (top)
    ─────┴───┴─────
         Base (on bed)
```

---

## Assembly

### Required Hardware

| Item | Quantity | Size | Purpose |
|------|----------|------|---------|
| M3 x 8mm screws | 4 | Phillips or hex | Base to top |
| M3 heat-set inserts | 4 | M3 x 5mm | Thread into plastic |
| M3 x 5mm screws | 2 | Phillips or hex | PCB mounting |

### Assembly Steps

1. **Insert heat-set inserts** (optional but recommended)
   - Heat soldering iron to 200°C
   - Press inserts into base holes
   - Let cool before handling

2. **Install electronics**
   - Place ESP32 in mounting slot
   - Secure with M3 x 5mm screws
   - Connect joystick and buttons

3. **Route wires**
   - Use wire channels in base
   - Ensure no pinching when closing

4. **Attach top plate**
   - Align joystick through hole
   - Insert M3 x 8mm screws
   - Tighten evenly (don't over-tighten)

5. **Install button caps and joystick knob**
   - Press-fit onto switches and joystick

---

## Enclosure Variants

### Standard (v1)

- Basic rectangular enclosure
- 120mm x 80mm x 45mm
- Single joystick + 2 buttons + 4 jacks
- Palm rest area

### Compact (Planned)

- Reduced footprint
- 90mm x 60mm x 35mm
- Single joystick + 2 buttons
- Minimal jacks

### Extended (Planned)

- Full-size layout
- 150mm x 100mm x 50mm
- Dual joystick option
- 8 buttons + 4 jacks
- OLED display cutout

### Mouth-Operated (Planned)

- Vertical orientation
- Gooseneck mount compatible
- Sip/puff tube integration
- Inspired by QuadStick

---

## Mounting Options

### 1/4-20 Camera Mount (Included)

Standard camera tripod thread, compatible with:
- RAM mounts
- Magic Arms
- Tripods
- Clamps

```
     ┌─────┐
     │Alpha│
     │Stick│
     └──┬──┘
        │  ← Mount adapter
     ───┴───
        ▼
   Camera mount / Magic Arm
```

### Wheelchair Tray Mount (Planned)

- Clamp-style attachment
- No drilling required
- Adjustable angle

### Desk Mount (Planned)

- C-clamp base
- Articulating arm
- Height adjustable

---

## Customization

### Modifying Designs

Source files are available in `models/source/` for:

- **FreeCAD** (free, open source)
- **Fusion 360** (free for personal use)

#### Common Modifications

1. **Joystick hole size** — Adjust for different joystick modules
2. **Button spacing** — Custom layouts for different hand sizes
3. **Palm rest angle** — Ergonomic adjustments
4. **Jack locations** — Move 3.5mm jacks to preferred side

### Color Options

Print in any color! Suggested schemes:

| Theme | Base | Top | Buttons |
|-------|------|-----|---------|
| Classic | Black | Black | Grey |
| Xbox | Black | Green | Green |
| PlayStation | Black | Blue | Blue |
| High Contrast | Black | White | Yellow |
| Fun | Any | Any | Mixed |

---

## Troubleshooting

### Warping

- **Cause:** Bed adhesion issues, drafts, cooling
- **Fix:** Clean bed, use brim, enclose printer, slow first layer

### Layer Separation

- **Cause:** Under-extrusion, low temperature
- **Fix:** Increase temp 5-10°C, check for clogs, calibrate flow

### Stringing

- **Cause:** Retraction settings, temperature too high
- **Fix:** Enable retraction (5-6mm), reduce temp, dry filament

### Poor Surface Finish

- **Cause:** Fast printing, layer height too high
- **Fix:** Slow down, use 0.16mm layers, check belt tension

### Parts Don't Fit

- **Cause:** Printer calibration, elephant foot
- **Fix:** Calibrate X/Y steps, adjust horizontal expansion in slicer

---

## Tips for Best Results

1. **Level your bed** — First layer adhesion is crucial
2. **Use fresh filament** — Moisture degrades quality
3. **Print a test cube** — Verify calibration before big prints
4. **Check belt tension** — Loose belts cause artifacts
5. **Clean nozzle** — Partial clogs cause under-extrusion
6. **Slow down** — Better quality at 40-50mm/s
7. **Let parts cool** — Remove from bed when fully cooled

---

## Print Time Estimates

| Part | Time (0.2mm) | Filament |
|------|--------------|----------|
| Base | 4-5 hours | ~50g |
| Top plate | 2-3 hours | ~30g |
| Mount adapter | 30 min | ~5g |
| Button caps (set) | 20 min | ~3g |
| Joystick knob | 15 min | ~2g |
| **Full set** | **~8 hours** | **~90g** |

---

## Community Designs

Share your custom designs with the community!

- **GitHub:** Submit PRs to `models/community/`
- **Thingiverse:** Tag with "AlphaStick"
- **Printables:** Tag with "AlphaStick"

---

## Next Steps

1. Download STL files from `models/stl/`
2. Slice with your preferred slicer (Cura, PrusaSlicer, etc.)
3. Print parts per this guide
4. Assemble per [ASSEMBLY.md](ASSEMBLY.md)
5. Share your build!

See [HARDWARE.md](HARDWARE.md) for electronics installation.
]]>
