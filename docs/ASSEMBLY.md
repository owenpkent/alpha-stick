# Assembly Guide

Step-by-step instructions for assembling Alpha Stick **V2** (sensor pod + main board).

> **Draft status:** the V2 mechanism is in Phase 0 bench validation ([TODO.md](../TODO.md)).
> Steps and dimensions here match [DESIGN_V2.md](DESIGN_V2.md) and [HARDWARE.md](HARDWARE.md)
> and will be finalized with photos once the first pod articles are built.

---

## Before You Begin

### Required Tools

| Tool | Purpose |
|------|---------|
| Soldering iron (30-40W) | Connectors, jacks |
| Solder (60/40 or lead-free) | Electrical joints |
| Flush cutters / wire strippers | Wire prep |
| Phillips screwdriver (#1) | M2/M3 screws |
| Cyanoacrylate glue (thin) | **All press-fit magnets get glued** |
| Multimeter | Continuity checks |

### Strongly Recommended

| Tool | Purpose |
|------|---------|
| Gram force gauge (0-50 g) | Verifying the 1-8 gf force range; cheap luggage-scale style works with a pulley |
| Feeler gauge or 1.5 mm drill shank | Setting the magnet-to-sensor gap |
| Digital scale (0.1 g) | Checking the 2.5 g moving-mass budget |
| Heat-set insert tip | M3 inserts in the base |

A US nickel is a calibrated 5.000 g reference mass; two quarters are ~11.3 g. Useful for
sanity-checking a gram gauge.

---

## Parts Checklist

### Sensor pod kit

- [ ] Pod PCB with 2x TMAG5273 (distinct I2C address variants) and JST-SH connector
- [ ] Sense magnet: N52 disc, D4 x 2 mm, **diametric**
- [ ] Ring magnet: N52, ~D10/D5 x 2 mm, axial
- [ ] Steel washer (M3, centering armature)
- [ ] 4 mm ball (steel or ceramic)
- [ ] PTFE cup (machined) or printed seat + PTFE tape
- [ ] Carbon stick tube (25/40/60 mm to taste)
- [ ] Printed: pod housing, adjuster carrier, stick hub, topper(s)
- [ ] Topper magnets: N52 D3 x 1 mm (one per topper + one in the hub)
- [ ] 6-pin JST-SH cable

### Main board kit

- [ ] Main PCB (assembled: ESP32-S3-MINI-1, USB-C, jacks, buttons, LEDs)
- [ ] Battery option: LiPo 503035 + slide switch (only if your board has the charger populated)
- [ ] Printed: base, top plate, button caps
- [ ] M3 x 8 mm screws (x4), M3 heat-set inserts (x4), M2 hardware for the pod (brass or nylon,
      **not steel**, near the sensors)

---

## Part A: Sensor Pod

The pod is the whole product; take your time here. Work on a clean bench, away from steel
shavings (magnets find them for you).

### A1. Mark the sense magnet axis

Diametric magnets have north on one *side*, not one face. Before installing:

1. Let the magnet snap sideways onto another magnet; the contact line marks the magnetic axis.
2. Mark that axis with a paint pen. It must end up aligned with the stick hub's index notch
   (the firmware calibration absorbs small errors, but starting aligned keeps the sweep clean).

### A2. Build the stick assembly

```
        carbon tube
            |
      +-----+-----+    stick hub (printed)
      | topper mag |   D3x1 in the top recess, glued
      |   tube     |   tube glued into the bore
      | washer     |   steel washer seated on the lower shoulder
      | sense mag  |   D4x2 diametric in the bottom recess, axis on the notch, glued
      +-----+-----+
            |
          (ball)       4 mm ball seats in the hub's bottom cup
```

1. Glue the carbon tube into the hub bore (CA, sparingly).
2. Seat the steel washer on its shoulder; a drop of CA.
3. Press the sense magnet into the bottom recess with the painted axis on the index notch; CA.
4. Press the topper magnet into the top recess; CA. **Check polarity against a topper before
   gluing** so toppers attract rather than repel.
5. Weigh the finished assembly: **target under 1.5 g** (leaves 1 g for a topper).

### A3. Build the pod housing

1. Press the PTFE cup (or printed seat lined with PTFE tape) into the housing center bore.
2. Glue the ring magnet into the adjuster carrier recess. Polarity: it must **attract** the
   steel washer (it will, either way, since the armature is steel; orientation only matters if
   you later swap the washer for a magnet).
3. Thread the carrier into the housing from below; stop at the middle detent.
4. Screw the pod PCB to the housing bosses with **brass or nylon M2** screws, sensors facing
   up, connector toward the cable channel.

### A4. Marry stick to pod and set the gap

1. Drop the ball into the PTFE cup, lower the stick hub onto it. The ring magnet should pull
   the assembly down into a seated, centered rest position.
2. Check the sense-magnet-to-sensor gap: **1.5 mm nominal** (1.0-2.5 mm usable). Adjust with
   the carrier; the printed stack is designed so the nominal detent lands near 1.5 mm.
3. Feel test: the stick should deflect with a fingertip brush, return crisply to center from
   every direction, and have zero perceptible grit. Grit means a contaminated cup: clean the
   ball and cup with IPA.
4. Force test (gauge or pulley + coins): full deflection in the 2-4 gf range at the middle
   detent, reaching roughly 1 gf backed out and 8 gf bottomed.

### A5. Connect and close

1. Plug the JST-SH cable, route through the channel, close the pod lid.
2. Snap a topper on. Verify it holds against an upside-down shake but releases with a pull.

---

## Part B: Main Board and Base

1. Install M3 heat-set inserts in the base (iron at 200-220 C, press gently, let cool).
2. Mount the main PCB on its standoffs (M3 x 5 mm).
3. Jacks and USB-C are board-mounted: align the base cutouts, no panel wiring needed.
4. Battery option: stick the LiPo in its bay with foam tape, route to the JST-PH, switch into
   its cutout. **Observe polarity; not all purchased LiPo pigtails match the board footprint.**
5. Seat the pod in its bay, connect the 6-pin cable to the main board.
6. Fit the top plate, start all four screws before tightening any, tighten evenly.
7. Press on button caps.

---

## Part C: Bring-Up Test

1. Connect USB. The device should enumerate as a composite HID gamepad/mouse/keyboard
   (Windows: check Device Manager, then "Set up USB game controllers").
2. Open a monitor for logs:

```powershell
cd C:\Users\Owen\dev\alpha-stick\firmware; idf.py -p COM5 monitor
```

3. Run the **guided calibration sweep** (hold both buttons at boot for config mode, connect to
   the `AlphaStick` AP, follow the calibration page): center hold, 8 compass points, full
   circle. See [CONFIGURATION.md](CONFIGURATION.md).
4. Verify in the web UI live view:
   - Full circle reaches the rim everywhere (no clipping, no flat spots)
   - Release from any direction settles within the center dot
   - Z-press registers at a light push, releases cleanly
5. Short each 3.5 mm jack tip-to-sleeve with a test cable: buttons fire.
6. If the dual-sensor health indicator shows disagreement, re-check magnet seating and gap.

---

## Magnet Handling and Safety

- **Glue every magnet.** Press-fits in printed plastic loosen with temperature cycles.
- Small magnets are an **ingestion hazard**; keep spares away from kids and pets.
- NdFeB is brittle: hand pressure only, never pliers or hammers.
- Keep magnets and tools with magnets away from the pod PCB during soldering; a magnetized
  iron tip near the sensors is a confusing afternoon.
- People with pacemakers/implants: follow the implant manufacturer's magnet distance guidance.
- LiPo: protected cells, correct polarity, no unattended charging.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| Stick flops, weak return | Carrier backed out too far, washer missing/unseated | Set carrier to middle detent; verify washer |
| Stick sits off-center | Cup or seat printed off-axis; magnet recess off-center | Reprint housing; check hub concentricity |
| Gritty feel | Debris in PTFE cup | IPA-clean ball and cup; rebuild on a clean bench |
| Axes weak or nonlinear in live view | Sense magnet not diametric, or axis badly misaligned | Verify magnet type (A1 step); re-seat on the notch |
| `cal_valid` stays false | Sweep not run, or gap far off nominal | Set 1.5 mm gap, re-run calibration |
| Z-click never fires | Preload too high (carrier bottomed) | Back the carrier off a detent or two |
| Dual-sensor fault | One sensor address wrong, or board flex | Check both sensors enumerate on I2C; reflow |
| USB unknown device | Cable is charge-only | Use a known data cable |
| Random resets on WiFi | Brownout on weak USB port | Use a powered port/hub |

---

## Maintenance

- Wipe the stick and toppers with a dry cloth; IPA for the ball/cup if feel degrades.
- Re-run the calibration sweep after any pod disassembly or magnet change.
- Re-check the force setting if strength needs change day to day: that is what the carrier
  detents are for, no tools needed.
- Firmware updates: web UI upload or `idf.py flash`; see [FIRMWARE.md](FIRMWARE.md).

---

## Next Steps

- [CONFIGURATION.md](CONFIGURATION.md): calibration, profiles, modes
- [PRINTING.md](PRINTING.md): printed parts and settings
- [DESIGN_V2.md](DESIGN_V2.md): why the mechanism works
