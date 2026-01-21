# Configuration Guide

How to configure Alpha Stick settings, sensitivity, and profiles.

---

## Accessing Configuration

### Web Configuration Portal

1. **Enter config mode:**
   - Hold Button 1 + Button 2 while plugging in USB
   - Or press the config button combo (future: configurable)

2. **Connect to WiFi:**
   - SSID: `AlphaStick`
   - Password: `configure`

3. **Open browser:**
   - Navigate to `http://192.168.4.1`

4. **Adjust settings** via the web UI

5. **Save and exit:**
   - Click "Save" to store settings
   - Click "Restart" to exit config mode

---

## Configuration Options

### Joystick Settings

#### Deadzone

The center area where small movements are ignored.

| Value | Effect | Best For |
|-------|--------|----------|
| 0% | No deadzone | Precise control, steady hands |
| 5-10% | Small deadzone | General use |
| 15-25% | Medium deadzone | Slight tremors |
| 30%+ | Large deadzone | Significant tremors |

```
      ┌─────────────────┐
      │                 │
      │    ┌───────┐    │
      │    │Deadzone│   │
      │    │ (no   │    │
      │    │output)│    │
      │    └───────┘    │
      │                 │
      └─────────────────┘
```

#### Sensitivity

How quickly the output reaches maximum value.

| Value | Effect | Best For |
|-------|--------|----------|
| 0.5 | Low sensitivity | Precise aiming |
| 1.0 | Normal | General use |
| 1.5 | High sensitivity | Fast movements |
| 2.0 | Very high | Limited range of motion |

#### Sensitivity Curve

How input maps to output across the range.

| Curve | Behavior | Best For |
|-------|----------|----------|
| Linear | Direct 1:1 mapping | Predictable control |
| Exponential | Slow start, fast end | Fine control near center |
| Logarithmic | Fast start, slow end | Quick response |
| S-Curve | Slow-fast-slow | Smooth acceleration |

#### Invert Axes

Flip the direction of X or Y axis.

- **Invert X:** Left ↔ Right swapped
- **Invert Y:** Up ↔ Down swapped

---

### Tremor Filtering

Smooths out unintended shaky movements.

| Value | Effect | Latency Added |
|-------|--------|---------------|
| Off (0) | No filtering | None |
| Low (0.3) | Slight smoothing | ~5ms |
| Medium (0.6) | Moderate smoothing | ~15ms |
| High (0.9) | Heavy smoothing | ~30ms |

**Note:** Higher values add input latency. Find a balance between smoothness and responsiveness.

---

### Button Configuration

#### Toggle vs Momentary

| Mode | Behavior | Use Case |
|------|----------|----------|
| Momentary | Active only while held | Triggers, actions |
| Toggle | Click to activate, click again to release | Sprint, aim-down-sights |

#### Debounce Time

Prevents multiple triggers from a single press.

| Value | Effect |
|-------|--------|
| 10ms | Fast response, may double-trigger |
| 30ms | Default, good balance |
| 50ms | Slower, reliable |
| 100ms | Very slow, for sensitive switches |

---

### 3.5mm Jack Configuration

Each jack can be mapped to a different button:

| Jack | Default Mapping | Configurable To |
|------|-----------------|-----------------|
| Jack 1 | Button 5 | Any button 1-16 |
| Jack 2 | Button 6 | Any button 1-16 |
| Jack 3 | Button 7 | Any button 1-16 |
| Jack 4 | Button 8 | Any button 1-16 |

Jacks can also be configured as:
- **Momentary** — Active while switch is pressed
- **Toggle** — Press to toggle on/off
- **Latched** — Requires specific release sequence

---

### Wireless Settings

#### Device Name

The name shown when pairing via Bluetooth.

- Default: `AlphaStick`
- Maximum length: 31 characters
- Allowed characters: A-Z, a-z, 0-9, space, hyphen

#### Bluetooth Enable/Disable

- **Enabled:** Device advertises and accepts BLE connections
- **Disabled:** Bluetooth radio off, saves power

---

## Profiles

### What is a Profile?

A profile stores all your settings:
- Deadzone
- Sensitivity
- Curves
- Button modes
- Tremor filter
- Axis inversions

Switch between profiles to quickly change configurations for different games or uses.

### Default Profiles

| Profile | Purpose |
|---------|---------|
| Default | Balanced settings for general use |
| Precise | Low sensitivity, small deadzone |
| Fast | High sensitivity, low filtering |
| Tremor | Large deadzone, heavy filtering |

### Managing Profiles

**Create new profile:**
1. Adjust settings as desired
2. Enter profile name
3. Click "Save as New Profile"

**Switch profiles:**
1. Select profile from dropdown
2. Click "Load"

**Delete profile:**
1. Select profile
2. Click "Delete"
3. Confirm deletion

---

## Configuration File

Settings are stored in ESP32 non-volatile storage (NVS). You can also export/import JSON:

### Export Settings

```json
{
  "profileName": "MySettings",
  "joystick": {
    "deadzone": 0.1,
    "sensitivity": 1.0,
    "curveType": "exponential",
    "invertX": false,
    "invertY": false
  },
  "filter": {
    "tremorFilter": 0.3,
    "debounceMs": 30
  },
  "buttons": {
    "toggleMode": [false, false, true, true, false, false, false, false]
  },
  "wireless": {
    "deviceName": "AlphaStick",
    "bleEnabled": true
  }
}
```

### Import Settings

1. Click "Import" in web UI
2. Paste JSON or upload file
3. Click "Apply"

---

## Xbox Adaptive Controller Mode

When connected to Xbox Adaptive Controller via 3.5mm:

### Joystick Output

Alpha Stick can output as a joystick axis to XAC:
- Connect to XAC's left stick or right stick 3.5mm jack
- Alpha Stick outputs analog voltage proportional to position

### Button Output

Each 3.5mm jack on Alpha Stick outputs a simple switch signal compatible with XAC's button inputs.

---

## Project Nimbus Integration

When using with [Project Nimbus](https://github.com/owenpkent/Project-Nimbus):

### Recommended Settings

| Setting | Value | Reason |
|---------|-------|--------|
| Deadzone | Match Nimbus | Consistent feel |
| Sensitivity | 1.0 | Let Nimbus handle curves |
| Tremor | Off | Nimbus has smoothing |

### Combined Use

1. Alpha Stick provides physical joystick input
2. Project Nimbus provides on-screen buttons and additional controls
3. Both output to vJoy/game together

---

## Troubleshooting Configuration

### Can't Access Web UI

| Issue | Solution |
|-------|----------|
| Can't find WiFi | Verify config mode active (LED blinks) |
| Can't connect | Check password: `configure` |
| Page won't load | Try http://192.168.4.1 (not https) |
| Page loads but no controls | Clear browser cache |

### Settings Not Saving

| Issue | Solution |
|-------|----------|
| Resets on reboot | Click "Save" before exiting |
| Corrupted settings | Use "Reset to Defaults" |
| NVS full | Delete unused profiles |

### Settings Not Taking Effect

| Issue | Solution |
|-------|----------|
| No change felt | Increase change amount |
| Opposite effect | Check invert settings |
| Delayed response | Reduce tremor filter |

---

## Quick Reference

### Default Settings

| Setting | Default Value |
|---------|---------------|
| Deadzone | 10% |
| Sensitivity | 1.0 |
| Curve | Linear |
| Tremor Filter | Off |
| Debounce | 30ms |
| Button Mode | Momentary |

### Button Combo Shortcuts

| Combo | Action |
|-------|--------|
| B1 + B2 (on boot) | Enter config mode |
| B1 + B2 (3 sec) | Reset to defaults |
| Joystick + B1 (boot) | Bluetooth pairing mode |

---

## Next Steps

- [FIRMWARE.md](FIRMWARE.md) — Update firmware for new features
- [HARDWARE.md](HARDWARE.md) — Hardware modifications
- [README.md](../README.md) — Full project overview
]]>
