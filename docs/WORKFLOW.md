# Development Workflow

How work happens on Alpha Stick: the iteration loops, the commands, and the
rules for keeping docs, CAD, and firmware from drifting apart. Conventions for
AI assistants live in [LLM_ONBOARDING.md](../LLM_ONBOARDING.md); this file is
about process.

---

## 1. Source-of-truth map

Every fact lives in exactly one place; everything else references it.

| Fact | Source of truth | Everything else |
|------|-----------------|-----------------|
| Why the design is what it is (budgets, trade studies, roadmap) | [DESIGN_V2.md](DESIGN_V2.md) | README summarizes |
| What to build (parts, pins, wiring, BOM) | [HARDWARE.md](HARDWARE.md) | ASSEMBLY consumes |
| Firmware architecture and wire formats | [FIRMWARE.md](FIRMWARE.md) + `firmware/components/as_aslink/include/aslink_frame.h` | DESIGN_V2 sec 9 mirrors |
| Pod geometry | `models/source/pod-v0.scad` (parameters at top) | STLs are generated artifacts |
| What to buy for Phase 0 | [PHASE0_PARTS.md](PHASE0_PARTS.md) | |
| What is done / next | [TODO.md](../TODO.md) | Constellation scrapes it |
| Measured reality | [BENCH_LOG.md](BENCH_LOG.md) | DESIGN_V2 estimates get corrected from it |

When a generated artifact (STL, sdkconfig, build output) disagrees with its
source, the source wins; regenerate, never hand-edit downstream.

---

## 2. The Phase 0 bench loop (where the project is now)

```
order parts -> print rig -> wire breakout -> measure -> log -> decide -> update docs
   (PHASE0_PARTS)  (models/)    (ASSEMBLY)     (below)  (BENCH_LOG)        (sec 5)
```

1. Work the checkboxes in [TODO.md](../TODO.md) Phase 0, roughly top to bottom;
   sensing bench and mechanics bench are independent and can interleave.
2. Every measurement session gets a dated entry in [BENCH_LOG.md](BENCH_LOG.md)
   (template at the top of that file): setup, numbers, verdict, what changes.
3. A bench item is done when its number beats the target in
   [DESIGN_V2.md](DESIGN_V2.md) section 10 exit criteria, or the design doc is
   updated to the measured reality with the reasoning.
4. Phase 0 ends with two decisions recorded in DESIGN_V2: the sensor
   (TMAG5273 vs MLX90393) and the pivot (printed dome vs steel ball). Then the
   real 20 x 20 pod (v1) gets designed from the logged numbers.

---

## 3. The CAD loop

Geometry lives in `models/source/pod-v0.scad`; STLs are exports.

```powershell
# 1. edit parameters at the top of the .scad (every dimension is named)
# 2. sanity-check the assembly cross-section
& "C:\Program Files\OpenSCAD\openscad.com" -o models\pod-v0-section.png --imgsize 1400,1000 --camera 0,0,0,70,0,300,60 --projection ortho -D "part=`"section`"" models\source\pod-v0.scad
# 3. regenerate the changed part (selector: hub|housing|base|carrier|topper|guide)
& "C:\Program Files\OpenSCAD\openscad.com" -o models\stl\pod\stick-hub-v0.stl -D "part=`"hub`"" models\source\pod-v0.scad
# 4. print per docs/PRINTING.md (pod parts at 0.12 mm; STLs are pre-flipped)
```

Rules:

- Commit the `.scad` and its regenerated STLs **in the same commit**, so the
  repo never offers a stale STL next to newer source.
- Fit problems get fixed in named parameters (`thr_clr`, `fit_mag`,
  `seat_clear`), not by scaling in the slicer; log the printer-specific value
  in BENCH_LOG so the default can be reconsidered after a few data points.
- The derived z-stack in the .scad echoes `pivot center z` and `tilt limit`
  on every render; if a change moves those unexpectedly, stop and re-check
  before printing.

---

## 4. The firmware loop

Requires an ESP-IDF 5.x PowerShell (Espressif Windows installer).

```powershell
cd C:\Users\Owen\dev\alpha-stick\firmware
idf.py set-target esp32s3      # once per checkout
idf.py build                   # compile
idf.py -p COM5 flash monitor   # flash via the UART connector, watch logs
idf.py menuconfig              # Alpha Stick menu: build profile, role, simulation
```

- **No hardware needed to start:** simulation mode synthesizes stick motion,
  so USB HID work happens on a bare devkit (`joy.cpl` shows circles).
- Grep for `VERIFY` before trusting bench numbers; each marks a value written
  from datasheet/API memory that needs checking against the real document.
  Remove the comment when verified, and log the correction if there was one.
- The component layout mirrors FIRMWARE.md's table; a new subsystem gets a new
  `components/as_*` directory, not a new file in an existing one.
- `aslink_frame.h` is shared with ATOS by vendoring: any change to it bumps
  `ASLINK_VERSION`, updates DESIGN_V2 section 9 and FIRMWARE.md, and gets
  copied to the ATOS gateway in the same sitting.
- Behavior differences between gaming and drive builds are expressed in
  Kconfig (`AS_BUILD_PROFILE`), never as runtime flags a profile could flip.

---

## 5. Change propagation (the anti-drift table)

When you change a thing on the left, touch everything on its row before
committing; the commit should contain the full row.

| Change | Must also update |
|--------|------------------|
| A requirement number (force, throw, latency) | DESIGN_V2 sec 1; README features if user-facing |
| Pin assignment | HARDWARE.md pin table; `sensing.cpp` / future `pins.h` constants |
| Pod dimension | `pod-v0.scad` parameter; regenerated STLs; HARDWARE.md if it changes the parts list |
| Profile fields | `as_pipeline/profile.h`; bump `kBlobVersion` in `as_config/config.cpp`; CONFIGURATION.md JSON example |
| AS-Link frame | `aslink_frame.h` + `ASLINK_VERSION`; DESIGN_V2 sec 9; FIRMWARE.md; ATOS gateway copy |
| Sensor choice (post-bench) | DESIGN_V2 decisions table; HARDWARE.md BOM; PHASE0_PARTS note; `as_sensing` driver |
| Print setting or orientation | PRINTING.md; models/README.md |
| Anything measured | BENCH_LOG.md entry, and correct the estimate it tested |

---

## 6. Tracking, status, Constellation

- [TODO.md](../TODO.md) is scraped by Constellation: keep checkbox syntax
  (`- [ ]` / `- [x]`), keep items actionable, check things off in the same
  commit as the work.
- README's `## Status` table is the dashboard summary; update a row when a
  phase meaningfully moves (not per-commit).
- New work items go into the current phase section; do not create a new phase
  without retiring the old one.

---

## 7. Git conventions

- Conventional commits: `feat:` / `fix:` / `docs:` / `refactor:` / `chore:`,
  present tense, one logical change per commit
  ([CONSTELLATION_INTEGRATION_GUIDE.md](../CONSTELLATION_INTEGRATION_GUIDE.md)
  has the full convention).
- Work lands on `main`; push when a loop iteration completes (docs row
  consistent, STLs regenerated, build green), not mid-iteration.
- Generated files that ship to users (STLs, preview PNGs) are committed;
  build trees (`firmware/build/`, `managed_components/`) are gitignored.

---

## 8. A worked example

"The carrier thread binds on my printer."

1. CAD loop: raise `thr_clr` 0.18 to 0.24 in `pod-v0.scad`, regenerate
   `adjuster-carrier-v0.stl`, reprint just the carrier.
2. It threads smoothly: BENCH_LOG entry (printer, filament, value that worked).
3. Propagation row "pod dimension": PRINTING.md already documents the knob, so
   no doc change; commit `.scad` + STL together:
   `fix: widen carrier thread clearance to 0.24 after bench fit`
4. TODO: nothing to check off (fit tuning is part of the print item).

One loop, one commit, no drift.
