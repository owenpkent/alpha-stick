# Configuration Guide

How to configure Alpha Stick **V2**: calibration, modes, sensitivity, filtering, and profiles.

One setting lives in hardware, not software: the **force adjuster** under the pod sets how
hard the stick is to move (1-8 gf), by turning the carrier between detents. Everything else
is firmware and lives in profiles.

---

## Accessing Configuration

### Web UI (full configuration)

1. **Enter config mode:** hold Button 1 + Button 2 while plugging in USB
2. **Connect to WiFi:** SSID `AlphaStick`, password `configure`
3. **Open** `http://192.168.4.1`
4. Adjust, **Save**, **Restart**

### WebSerial (no WiFi needed)

The USB CDC interface speaks the same JSON config protocol. Any WebSerial-capable browser page
or a serial terminal can read/write settings while the device keeps working. Good for quick
tweaks and scripting.

---

## Calibration and Tare

Do this once per build, and again after any pod disassembly.

### Guided calibration sweep

Web UI -> Calibration -> Start. You will be asked to:

1. Leave the stick at rest (center hold)
2. Deflect to 8 compass points
3. Sweep one slow full circle at the rim

This fits the field-to-angle map. The live view should then reach the rim in all directions
with no flat spots. `cal_valid` shows green when the map is good.

### Mount tare (gravity bias)

If the unit is mounted tilted or vertical (chin boom, wheelchair rail), the stick's own weight
biases it. In the profile, run **Mount tare** with the unit installed at its real angle; the
offset is stored per profile, so a "desk" profile and a "chin" profile can coexist.

### Center tare rules

- Manual tare: hold Button 1 for 3 s (stick untouched), or web UI -> Tare
- Automatic tare happens only at boot, only if the stick has been still and near the stored
  center for 3 s. It never re-tares silently during use.

Tilted mounts: also consider a stiffer force setting (4-6 gf detents) so the stick returns
against gravity.

---

## Modes

| Mode | What streams | Switch |
|------|--------------|--------|
| Gamepad | 16-bit stick + buttons | hold Button 2 for 3 s cycles modes (configurable) |
| Mouse | pointer velocity, clicks, scroll | |
| Keyboard | 8-way WASD/arrows | |
| Dual | gamepad + mouse together | |
| ATOS input | AS-Link frames, HID silent | only via profile (deliberate) |

The device never re-enumerates on a mode switch; the OS sees one stable composite device.
The haptic option plays a distinct tick per mode so you know where you landed without looking.

---

## Stick Settings

### Deadzone (radial)

V2 deadzones are radial (a circle around center), which preserves diagonals.

| Inner | Effect | Best for |
|-------|--------|----------|
| 0-2% | Minimal | Steady input, precision work |
| 4% (default) | Small | General use |
| 8-15% | Medium | Mild tremor |
| 20%+ | Large | Significant tremor; pair with filtering |

`outer` (default 98%) sets where full deflection saturates, so the rim is reliably reachable.

### Response curve

| Curve | Behavior | Best for |
|-------|----------|----------|
| Linear | 1:1 | Predictable control |
| Expo (default 0.4) | Fine near center, fast at rim | Aiming + movement in one stick |
| Log | Fast near center | Quick flicks, menus |
| S-curve | Slow-fast-slow | Smooth vehicle control |

Curves are configured **per mode**: the gamepad curve and the mouse curve are independent.

### Invert / rotate

- Invert X or Y per profile
- `rotation_deg` rotates both axes for angled mounting (set during mount tare)

---

## Filtering (tremor)

V2 uses a **1-euro filter**: smoothing is strong when the stick moves slowly (tremor at rest)
and backs off when you move deliberately (no aiming lag). Two knobs:

| Preset | min_cutoff | beta | Feel |
|--------|-----------|------|------|
| Off | filter bypassed | | Raw |
| Light (default) | 1.0 | 0.02 | Invisible smoothing |
| Medium | 0.5 | 0.01 | Noticeable steadying |
| Heavy | 0.25 | 0.005 | Strong steadying, slight lag at slow speeds |

Plus an optional **notch filter** (3-12 Hz) targeting rhythmic intention tremor; set the
frequency to the tremor, leave 0 to disable.

Start with deadzone, add Light/Medium filtering, and only then reach for the notch.

---

## Mouse Mode Settings

| Setting | Default | Notes |
|---------|---------|-------|
| `max_px_s` | 900 | Pointer speed at full deflection |
| Precision toggle | jack-assignable | Halves speed while active |
| Scroll toggle | jack-assignable | Stick becomes scroll |
| Dwell click | on, 2% radius, 600 ms | Pointer held still fires a click |
| Dwell action | left click | left/right/double configurable |
| Z-press | left click | The stick's own push-to-click |

Dwell click exists so the mouse is fully usable with **zero** clicking force: point, rest,
click happens.

---

## Buttons, Jacks, Sip/Puff

### Sources and mappings

Any source can map to any output in the active mode:

| Source | Notes |
|--------|-------|
| Button 1, Button 2 | On-device tactile |
| Jack 1-4 | External switches, XAC-style |
| Z-press | Push the stick down (<5 gf) |
| Sip/puff | Four thresholds: hard sip, soft sip, soft puff, hard puff |

### Behaviors

| Behavior | Effect |
|----------|--------|
| Momentary | Active while held |
| Toggle | Press to latch, press to release |
| Debounce | 10-100 ms, default 30 |

### Sip/puff thresholds

Calibrate in the web UI: rest, soft sip, hard sip, soft puff, hard puff. Hysteresis between
soft/hard prevents flutter. The four logical buttons then map like any other source.

---

## Wireless

### Bluetooth

- Pairing: hold Button 1 + Button 2 for 5 s while running (LED pulses blue)
- The device keeps **3 bonds**; the same combo short-press cycles to the next bonded host
- Clear all bonds: hold Button 1 + Button 2 for 10 s
- Device name: default `AlphaStick`, configurable

### ESP-NOW (dongle / ATOS)

Pair once via the web UI (the stick and the receiving peer exchange MACs). Latency is in the
2-4 ms class; preferred over BLE for fast games when running wireless.

---

## Profiles

A profile stores everything above (mode, calibration tare, deadzone, curves, filters,
mappings, wireless preferences). Switch via web UI, a mapped jack, or a button combo.

```json
{
  "name": "default",
  "mode": "gamepad",
  "mount": { "rotation_deg": 0, "gravity_tare": [0.0, 0.0] },
  "deadzone": { "type": "radial", "inner": 0.04, "outer": 0.98 },
  "curve": { "type": "expo", "gain": 1.0, "expo": 0.4 },
  "filter": { "one_euro": { "min_cutoff": 1.0, "beta": 0.02 }, "notch_hz": 0 },
  "mouse": { "max_px_s": 900, "dwell": { "enabled": true, "radius_pct": 2, "ms": 600 } },
  "buttons": { "jack1": "A", "jack2": "B", "zclick": "LMB",
               "sip_hard": "RMB", "puff_hard": "LMB" }
}
```

Export/import as JSON in the web UI. The schema deliberately overlaps with Project Nimbus so
curves and deadzones can be shared between the two.

### Starter profiles

| Profile | Setup |
|---------|-------|
| Default | 4% deadzone, expo 0.4, Light filter |
| Precise | 2% deadzone, linear, Light filter, mouse `max_px_s` 500 |
| Tremor | 12% deadzone, expo, Medium filter + notch if rhythmic |
| Chin mount | mount tare set, force detent 5 gf (hardware), S-curve |

---

## Platform Notes

### Xbox Adaptive Controller

Connect Alpha Stick to an **XAC USB port** (left or right); it appears as that side's stick
(X1/X2). The 3.5 mm jacks on Alpha Stick are *inputs* for your own switches; they do not plug
into the XAC. Use the gamepad mode default profile; let the XAC own button mapping to Xbox.

### PlayStation Access

Switch-type accessories via the Access controller's own 3.5 mm AUX ports remain the sanctioned
path for buttons; analog passthrough is not assumed. For PC play, use Alpha Stick directly.

### Project Nimbus

| Setting | Value | Reason |
|---------|-------|--------|
| Deadzone | small (2-4%) | Nimbus applies its own |
| Curve | linear, gain 1.0 | Let Nimbus shape response |
| Filter | Light | Avoid double-smoothing |

---

## Button Combo Quick Reference (defaults, remappable)

| Combo | Action |
|-------|--------|
| B1 + B2 while plugging in | Config mode (WiFi AP) |
| Hold B1 3 s | Center tare |
| Hold B2 3 s | Cycle mode |
| B1 + B2 5 s (running) | BLE pairing / next bond |
| B1 + B2 10 s | Clear BLE bonds |

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Can't find `AlphaStick` WiFi | Re-enter config mode (B1+B2 at plug-in); LED should blink |
| Live view clips before the rim | Re-run calibration sweep |
| Center drifts on a tilted mount | Run mount tare at the installed angle; consider a stiffer force detent |
| Pointer too twitchy in mouse mode | Lower `max_px_s` before adding filtering |
| Dwell clicks fire accidentally | Increase radius_pct or ms; or disable and use Z-press |
| Settings reset on reboot | Save before restart; check NVS not full (delete unused profiles) |
| Stick feels too heavy/light | That is the hardware force adjuster, not software: turn the carrier |

---

## Next Steps

- [ASSEMBLY.md](ASSEMBLY.md): build and first-run calibration
- [FIRMWARE.md](FIRMWARE.md): updating firmware, build profiles
- [DESIGN_V2.md](DESIGN_V2.md): design rationale
