# Models

Printable parts for Alpha Stick V2. Print settings live in
[docs/PRINTING.md](../docs/PRINTING.md); the design rationale in
[docs/DESIGN_V2.md](../docs/DESIGN_V2.md).

## Tetra II flexure: the primary pivot mechanism

Alpha Stick V2 pivots on the **Tetra II spherical flexure joint** in
[`tetra2-flexure/`](tetra2-flexure/). It is a bearing-free compliant gimbal:
nested tetrahedron blade flexures give 3-DOF rotation about a remote centre that
floats ~50 mm out from the joint, with **no sliding surfaces** — so no friction,
no backlash, no break-away force, and the blades themselves provide the elastic
return to centre (no centering magnet, no spring, no threaded force adjuster).
The diametric sense magnet rides the moving platform and the dual TMAG5273 Hall
sensors read tilt exactly as before; only the pivot-and-return mechanism changes.

![Tetra II flexure — isometric](tetra2-flexure/preview-iso.png)
![Tetra II flexure — front](tetra2-flexure/preview-front.png)
![Tetra II flexure — side](tetra2-flexure/preview-side.png)

| File | What it is |
|------|------------|
| [`tetra2-flexure/Tetra2.STEP`](tetra2-flexure/Tetra2.STEP) | source CAD (B-rep); scale, thicken blades, or cut a stick mount in FreeCAD / Fusion / SolidWorks |
| [`tetra2-flexure/Tetra2.STL`](tetra2-flexure/Tetra2.STL) | ready-to-slice mesh of the same part |
| [`tetra2-flexure/README.md`](tetra2-flexure/README.md) | paper analysis, printing notes, and CC-BY attribution |

This is Jelle Rommers' design and carries an upstream **CC-BY** licence (credit
required), separate from the repo's MIT / CERN-OHL-P work — see the folder
README. Stiffness (= the force you feel) is hard to predict for the real
tetrahedron element and is set by blade geometry, material, and scale, so it is
a **Phase 0 bench measurement**, not a hand-calculated figure. A printed PLA
joint is fine for feel studies but creeps and fatigues under daily use; PETG or
a printed-then-cast/metal version are the follow-ups.

## Pod v0: the ball-pivot bench rig (alternative mechanism)

The pod parts below build the **alternative** ball-in-PTFE-cup mechanism with
magnetic centering — the original V2 path, kept as a fallback and as the bench
rig that characterises the dual-Hall sensing independent of the pivot choice
(see [docs/HARDWARE.md](../docs/HARDWARE.md#alternative-mechanism-ball-in-ptfe-pod)).

`source/pod-v0.scad` (OpenSCAD, fully parametric) generates the rig that
validates the V2 mechanism using the parts from
[docs/PHASE0_PARTS.md](../docs/PHASE0_PARTS.md). It is sized around the
**SparkFun Qwiic TMAG5273 breakout** (25.4 mm square), not the final 20 x 20 mm
pod PCB; v0 exists to produce the numbers the real pod gets sized from.

![Assembly cross-section](pod-v0-section.png)

| Part | STL | What it does |
|------|-----|--------------|
| Stick hub | `stl/pod/stick-hub-v0.stl` | Printed dome pivot, D4x2 diametric magnet pocket (0.2 mm proud), anti-yaw pins, 2.6 mm carbon tube bore |
| Seat housing | `stl/pod/seat-housing-v0.stl` | PTFE-tape-lined spherical socket on a bridge over the breakout, anti-yaw slots, corner posts |
| Bench base | `stl/pod/bench-base-v0.stl` | Breakout pocket (PCB top lands flush), M12x0.75 threaded boss for the carrier, Qwiic cable reliefs |
| Adjuster carrier | `stl/pod/adjuster-carrier-v0.stl` | M12x0.75 thread, top pocket for a D10 ring magnet or M5 washer stack, grip wheel |
| Ball topper | `stl/toppers/topper-ball-v0.stl` | 8 mm ball, press-fits the carbon tube |
| Thread guide | `stl/pod/thread-guide-v0.stl` | String guide arm for the gram-scale force rig, notch near the 40 mm grip height |

### How the mechanism works in v0

The hub's printed dome (R6) rests in the housing socket lined with PTFE tape.
The sense magnet in the dome flat sits 1.5 mm above the TMAG5273 (the stack
arithmetic is derived, commented, and echoed in the .scad). The carrier rises
through the base from below; whatever sits in its pocket (ring magnet, washer
stack, or steel disc) attracts the sense magnet downward: that one attraction
is simultaneously the pivot preload and the centering force. Screwing the
carrier up or down sweeps the magnet-to-armature gap from about 4.4 to 9 mm,
which is the 1-8 gf force adjustment under test. Anti-yaw pins in collar slots
stop the hub from spinning (a diametric magnet reads yaw as direction change,
so this is required, not cosmetic). Tilt limit with default numbers: ~7 deg.

### v0 deviations from the production pod (docs/HARDWARE.md)

- Printed dome instead of a discrete steel ball; the bench compares feel and
  breakout force, and the steel-ball variant returns if PTFE-on-PETG loses.
- No steel washer on the hub; centering acts directly on the sense magnet.
- No carrier detents; printed-thread friction holds position at gram loads.
- ASSEMBLY.md describes the production pod build; for v0, assembly is: glue
  magnet in hub, tape the socket, drop hub in housing, screw housing to base
  over the breakout, thread the carrier in from below.

### Print notes

- Hub and housing at 0.12 mm; both export pre-flipped to their print
  orientation (hub neck-down so the dome prints as a smooth top surface;
  housing collar-down so the socket bowl faces up). No supports.
- The hub's anti-yaw pins print as short horizontal cantilevers; a little sag
  is fine (the slots have 0.6 mm total clearance).
- Base prints boss-down as exported, brim recommended.
- Thread binds? Adjust `thr_clr` (default 0.18) and reprint just the carrier.

### Regenerating

Edit parameters at the top of `source/pod-v0.scad`, then:

```powershell
& "C:\Program Files\OpenSCAD\openscad.com" -o models\stl\pod\stick-hub-v0.stl -D "part=`"hub`"" models\source\pod-v0.scad
```

Part selector values: `hub`, `housing`, `base`, `carrier`, `topper`, `guide`,
`plate` (all parts in print orientation), `section` (assembly cross-section).

## Layout

```
models/
+-- tetra2-flexure/  primary pivot: Tetra II flexure STEP + STL (third-party, CC-BY)
+-- source/          pod-v0.scad (parametric source of truth for the alternative pod)
+-- stl/             ready-to-print pod exports (regenerate after editing source)
+-- step/            reserved for the production pod (FreeCAD/Fusion era)
+-- community/       community-contributed toppers and bodies (created on first PR)
```

Third-party parts (currently the Tetra II flexure) keep their upstream licence
in their own folder README; everything else is the repo's MIT / CERN-OHL-P work.
