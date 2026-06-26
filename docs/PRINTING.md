# 3D Printing Guide

Printed parts for Alpha Stick **V2**: the sensor pod mechanism, the bodies it drops into, and
toppers. Geometry is being finalized through Phase 0 bench work ([TODO.md](../TODO.md));
settings below are the targets the parts are designed around.

> **Primary pivot — the Tetra II flexure** prints as one piece from
> [`models/tetra2-flexure/`](../models/tetra2-flexure/Tetra2.STL): PLA works for a demonstrator,
> but **prefer PETG** for creep and fatigue life; ~0.7 mm blade walls (two slightly overlapping
> perimeters on a 0.4 mm nozzle); the Tetra2 mesh prints without supports. The pod-mechanism
> settings below are for the **alternative** ball-in-PTFE pod (housing, carrier, hub).

---

## Overview

V2 splits printing into three groups:

1. **Pod mechanism parts** (precision matters): pod housing, adjuster carrier, stick hub
2. **Bodies** (forgiving): desktop base, top plate, thumb puck, chin-boom hardware
3. **Toppers** (mass matters): ball, dish, lip bar, extensions

```
models/
+-- source/
|   +-- pod-v0.scad                   # parametric source for all pod v0 parts
+-- stl/
|   +-- pod/
|   |   +-- seat-housing-v0.stl       # PTFE-tape socket, anti-yaw slots, posts
|   |   +-- bench-base-v0.stl         # Qwiic breakout pocket, M12x0.75 boss
|   |   +-- adjuster-carrier-v0.stl   # ring magnet pocket, M12x0.75 thread
|   |   +-- stick-hub-v0.stl          # dome pivot, magnet pocket, tube bore
|   |   +-- thread-guide-v0.stl       # force-rig string guide
|   +-- bodies/                       # (production era, after Phase 0)
|   |   +-- base-desktop-v2.stl       # main PCB + pod bay + jack/USB cutouts
|   |   +-- top-plate-v2.stl
|   |   +-- thumb-puck-v2.stl         # 30 mm pod-only puck
|   |   +-- mount-adapter.stl         # 1/4-20 + AMPS
|   |   +-- button-cap-flat.stl
|   |   +-- button-cap-dome.stl
|   +-- toppers/
|       +-- topper-ball-8mm.stl
|       +-- topper-dish-14mm.stl
|       +-- topper-lip-bar.stl
|       +-- topper-ext-20.stl
|       +-- topper-ext-40.stl
+-- step/                             # for modification
+-- source/                           # FreeCAD/Fusion360
```

---

## Per-Part Settings (the table that matters)

| Part | Layer | Walls | Infill | Material | Notes |
|------|-------|-------|--------|----------|-------|
| Pod housing | 0.12 mm | 4 | 40% | PETG/PCTG | Thread + cup bore need the fine layers |
| Adjuster carrier | 0.12 mm | 4 | 40% | PETG/PCTG | Test-fit thread; see thread tuning below |
| Stick hub | 0.12 mm | 100% solid | | PETG | Tiny part; solid for magnet retention |
| Desktop base / top plate | 0.2 mm | 3 | 20% | PLA/PETG | Standard settings |
| Thumb puck | 0.16 mm | 3 | 25% | PETG | |
| Mount adapter | 0.16 mm | 4 | 40% | PETG | Thread strength |
| Button caps | 0.12 mm | | 100% | PLA/PETG/TPU | |
| **Toppers** | 0.16 mm | 2 | 5% | PLA | **Hollow on purpose: each topper must weigh under 1 g** |

### The topper mass budget is real

The whole moving assembly has a 2.5 g budget ([DESIGN_V2.md](DESIGN_V2.md) section 3); heavy
toppers make the stick sag on tilted mounts. Print toppers light (2 walls, 5% infill), weigh
them, and write the mass on the bag. When designing custom toppers, hollow them.

### Thread tuning (housing + carrier)

The M12x0.75 adjuster thread is the only printed precision fit:

- Print both parts on the same printer with the same profile
- If the carrier binds: add 0.05-0.1 mm horizontal expansion compensation (slicer) and reprint
  the carrier only
- The detent spring finger is printed in place; if detents feel mushy at 0.2 mm layers, you
  skipped the 0.12 mm requirement
- A dry PTFE-based lubricant (never grease: it migrates to the pivot) is acceptable on the
  thread

### Pivot seat note

The reference pivot is a machined PTFE cup pressed into the housing. The budget path is a
printed seat in **PCTG/PETG lined with PTFE tape**: print the seat bore 0.2 mm oversize to
leave room for the tape layer. PLA works but creeps under the preload over months; prefer
PETG-class materials anywhere the magnets load the plastic.

---

## Materials

| Material | Use in V2 | Notes |
|----------|-----------|-------|
| PLA | Bodies, toppers | Easy, fine for everything that is not preloaded |
| PETG / PCTG | Pod mechanism, anything magnet-loaded, travel builds | Creep resistance is why |
| TPU | Button caps, grippy topper sleeves | Optional |
| ABS/ASA | Not needed | Skip unless the unit lives in a hot car |

**Mouthpiece caution:** FDM prints are porous and hard to sanitize. For mouth-operated use,
the printed part should hold a **commercial food-safe mouthpiece** (CamelBak-style bite valve
or QuadStick-compatible mouthpiece) rather than being the mouthpiece itself.

---

## General Print Settings

| Setting | Value | Notes |
|---------|-------|-------|
| Layer height | 0.2 mm bodies, 0.12 mm pod parts | Per-part table above wins |
| Walls | 3 (4 for threads) | |
| Top/bottom | 4 | |
| Speed | 50 mm/s, slower for pod parts | |
| Supports | None | Parts are designed support-free |
| Adhesion | Brim optional | |

Minimum printer: 150 x 150 x 100 mm (Ender 3 class). The largest part (base) is ~120 x 80 mm.

---

## Print Orientation

```
Pod housing: cup bore UP          Carrier: thread UP, magnet recess down
   +---------+                        +-----+
   | o cup o |                        |#####|  <- thread
   |  bosses |                        +--+--+
   +---------+                        |     |
   bed                                bed

Stick hub: neck DOWN (dome up,    Base: flat, bottom down
   smooth pivot surface on top)   Top plate: top face DOWN (smooth)
                                  Toppers: stem down, no supports
Pod v0 STLs export pre-flipped to their print orientation
(see models/README.md), so slice them as they load.
```

Orientation matters most on the hub and housing: the magnet recesses and cup bore should be
printed as horizontal circles (around the Z axis) so they come out round.

---

## Bodies: One Pod, Many Builds

| Body | Status | Purpose |
|------|--------|---------|
| Desktop base | V2 default | Main PCB + pod, palm-friendly, jack/USB access |
| Thumb puck | V2 | Pod-only 30 mm puck on a cable, for tray mounting |
| Chin boom pod | Planned | Pod + cable on a gooseneck/boom clamp |
| Mouth-stick frame | Planned | Vertical frame, sip/puff tube routing, bite-valve holder |
| Dual-stick base | Backlog | Two pods, one main board |

All bodies accept the same pod and the same 6-pin cable; print the body your mounting
situation needs.

---

## Mounting

- Base and puck carry the **AMPS hole pattern** plus a **1/4-20 brass insert**: RAM mounts,
  Magic Arms, tripods, wheelchair tray clamps
- Set the insert with a soldering iron at ~220 C, flush or slightly proud, let cool fully
  before loading

---

## Hardware for Printed Parts

| Item | Qty | Use |
|------|-----|-----|
| M3 x 8 mm + heat-set inserts | 4 | Base to top plate |
| M3 x 5 mm | 2-4 | PCB standoffs |
| M2 brass or nylon screws | 2-3 | Pod PCB (**non-magnetic, near sensors**) |
| 1/4-20 brass insert | 1 | Mounting |

---

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| Carrier thread binds | Over-extrusion / no compensation | Horizontal expansion -0.05 to -0.1 mm, reprint carrier |
| Detents feel mushy | Layer height too coarse | 0.12 mm on housing + carrier |
| Stick sits off-center | Cup bore or magnet recess out of round | Reprint with part oriented per the diagrams; check belts |
| Magnets fall out | Press-fit only | Always glue (thin CA) |
| Warping | Adhesion/drafts | Clean bed, brim, slow first layer |
| Stringing on PETG | Temp/retraction | Reduce 5-10 C, dry filament |
| Parts don't fit | Elephant foot | First-layer squish down, or chamfered reprint |

---

## Print Time Estimates

| Part | Time | Filament |
|------|------|----------|
| Pod set (housing + carrier + hub) | ~2 h | ~15 g |
| Desktop base | 4-5 h | ~50 g |
| Top plate | 2-3 h | ~30 g |
| Topper set | ~45 min | ~6 g |
| Mount adapter | 30 min | ~5 g |
| **Full desktop build** | **~9 h** | **~105 g** |

---

## Community Designs

Custom toppers and bodies are the easiest, highest-value contributions:

- **GitHub:** PRs to `models/community/` (include the printed mass of toppers)
- **Printables/Thingiverse:** tag `AlphaStick`

---

## Next Steps

1. Print a pod set at 0.12 mm and the desktop base at 0.2 mm
2. Assemble per [ASSEMBLY.md](ASSEMBLY.md)
3. Calibrate per [CONFIGURATION.md](CONFIGURATION.md)

See [HARDWARE.md](HARDWARE.md) for the electronics that go inside.
