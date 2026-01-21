# Assembly Guide

Step-by-step instructions for assembling Alpha Stick.

---

## Before You Begin

### Required Tools

| Tool | Purpose |
|------|---------|
| Soldering iron (30-40W) | Wire connections |
| Solder (60/40 or lead-free) | Electrical joints |
| Wire strippers | Preparing wires |
| Phillips screwdriver (#1) | M3 screws |
| Flush cutters | Trimming wires |
| Multimeter | Testing connections |
| Heat gun or lighter | Heat shrink tubing |

### Optional Tools

| Tool | Purpose |
|------|---------|
| Helping hands | Hold parts while soldering |
| Soldering iron with heat-set tip | Installing heat-set inserts |
| Hot glue gun | Strain relief |
| Label maker | Wire identification |

---

## Parts Checklist

Before assembly, verify you have all components:

### Electronics

- [ ] ESP32-S3 DevKitC (or compatible)
- [ ] Analog joystick module
- [ ] Tactile buttons (x2)
- [ ] 3.5mm mono jacks (x4)
- [ ] USB-C breakout (if not using devkit USB)
- [ ] Wires (24-28 AWG, various colors)
- [ ] Heat shrink tubing

### 3D Printed Parts

- [ ] Base enclosure
- [ ] Top plate
- [ ] Mount adapter (optional)
- [ ] Button caps (x2)
- [ ] Joystick knob

### Hardware

- [ ] M3 x 8mm screws (x4)
- [ ] M3 heat-set inserts (x4) — optional
- [ ] M3 x 5mm screws (x2) — PCB mounting

---

## Assembly Steps

### Step 1: Prepare the Base

**Install heat-set inserts (recommended):**

1. Heat soldering iron to 200-220°C
2. Place insert on hole, threading facing up
3. Apply gentle pressure with iron tip
4. Insert sinks flush with surface
5. Let cool 30 seconds before handling
6. Repeat for all 4 corners

```
       ↓ Iron
    ┌──┴──┐
    │ ░░░ │ ← Insert
    └─────┘
    ╔═════╗
    ║     ║ ← Plastic base
```

**Alternative (no inserts):** Screw directly into plastic. Works but may strip over time.

---

### Step 2: Prepare Wires

Cut and strip wires for each connection:

| Connection | Length | Color (suggested) |
|------------|--------|-------------------|
| Joystick VCC | 60mm | Red |
| Joystick GND | 60mm | Black |
| Joystick X | 60mm | Yellow |
| Joystick Y | 60mm | Orange |
| Joystick button | 60mm | Blue |
| Button 1 | 50mm | Green |
| Button 2 | 50mm | Green |
| Jack 1-4 (x2 each) | 40mm | White |

Strip 3-4mm from each end.

---

### Step 3: Solder Joystick

**Joystick module connections:**

```
Joystick Module          ESP32-S3
  ┌─────────┐
  │  VCC ───┼──────────── 3.3V
  │  GND ───┼──────────── GND
  │  VRx ───┼──────────── GPIO1
  │  VRy ───┼──────────── GPIO2
  │  SW  ───┼──────────── GPIO37
  └─────────┘
```

1. Solder wires to joystick module pads
2. Add heat shrink to prevent shorts
3. Route wires through base channel

---

### Step 4: Solder Buttons

**Button wiring (each button):**

```
    ┌───┐
    │   │━━━━━ GPIO (pullup enabled)
    │ ○ │
    │   │━━━━━ GND
    └───┘
```

1. Solder one leg to GPIO wire
2. Solder opposite leg to GND wire
3. Button orientation doesn't matter

**GPIO assignments:**
- Button 1 → GPIO39
- Button 2 → GPIO38

---

### Step 5: Solder 3.5mm Jacks

**Jack wiring:**

```
3.5mm Mono Jack
    ┌─────────┐
    │ Tip   ──┼──── GPIO (with external pullup)
    │         │
    │ Sleeve ─┼──── GND
    └─────────┘
```

Each jack needs:
- Tip → GPIO (with 10K pullup resistor to 3.3V)
- Sleeve → GND

**GPIO assignments:**
- Jack 1 → GPIO35
- Jack 2 → GPIO34
- Jack 3 → GPIO33
- Jack 4 → GPIO26

**Add pullup resistors:**
```
3.3V ──┬── 10K ──┬── GPIO
       │         │
       └─────────┴── Jack Tip
```

Or use internal pullups in firmware (less reliable with long cables).

---

### Step 6: Connect to ESP32

**Soldering to DevKit:**

Use the pin headers on the ESP32 DevKit. You can:

1. **Solder directly** — Permanent but reliable
2. **Use DuPont connectors** — Removable but can disconnect
3. **Custom PCB** — Best for production (future)

**Connection summary:**

| ESP32 Pin | Function |
|-----------|----------|
| 3.3V | Joystick power |
| GND | Common ground |
| GPIO1 | Joystick X |
| GPIO2 | Joystick Y |
| GPIO37 | Joystick button |
| GPIO38 | Button 2 |
| GPIO39 | Button 1 |
| GPIO35 | Jack 1 |
| GPIO34 | Jack 2 |
| GPIO33 | Jack 3 |
| GPIO26 | Jack 4 |

---

### Step 7: Test Electronics

**Before final assembly, test everything:**

1. Connect USB to computer
2. Open Arduino Serial Monitor (115200 baud)
3. Verify joystick readings change when moved
4. Verify buttons register as pressed
5. Test each 3.5mm jack with a test cable

**Test script:**
```cpp
void setup() {
    Serial.begin(115200);
    pinMode(37, INPUT_PULLUP);  // Joystick button
    pinMode(38, INPUT_PULLUP);  // Button 2
    pinMode(39, INPUT_PULLUP);  // Button 1
}

void loop() {
    Serial.printf("X:%d Y:%d BTN:%d B1:%d B2:%d\n",
        analogRead(1),
        analogRead(2),
        digitalRead(37),
        digitalRead(39),
        digitalRead(38)
    );
    delay(100);
}
```

---

### Step 8: Mount Components in Base

**Order of installation:**

1. **Joystick** — Align with hole, secure with screws or hot glue
2. **3.5mm jacks** — Insert from inside, nut on outside
3. **Buttons** — Press-fit into mounting holes
4. **ESP32** — Secure with M3 x 5mm screws

**Wire routing:**

```
┌─────────────────────────┐
│  ┌───┐     ┌───────┐    │
│  │ J │     │ESP32  │    │
│  │ O │─────│       │    │
│  │ Y │     │       │    │
│  └───┘     └───────┘    │
│                         │
│  [B1] [B2]    ○ ○ ○ ○   │
│               Jacks     │
└─────────────────────────┘
```

Keep wires flat in channels to prevent pinching.

---

### Step 9: Final Assembly

1. **Check clearances** — No wires crossing screw holes
2. **Test fit top plate** — Verify joystick aligns
3. **Install top plate** — Start screws in all corners before tightening
4. **Tighten evenly** — Alternate corners, don't over-tighten
5. **Install button caps** — Press onto tactile buttons
6. **Install joystick knob** — Press-fit or screw onto joystick shaft

---

### Step 10: Final Test

1. Connect USB to computer
2. Verify appears as gamepad in device manager
3. Open "Set up USB game controllers" in Windows
4. Test all axes and buttons
5. Verify 3.5mm jacks trigger buttons when shorted

**Congratulations! Your Alpha Stick is complete!**

---

## Troubleshooting

### Joystick Not Responding

| Symptom | Cause | Fix |
|---------|-------|-----|
| No reading | Wiring issue | Check VCC/GND connections |
| Stuck at 0 or 4095 | Axis wire disconnected | Resolder connection |
| Jittery values | Noise | Add 100nF cap near ADC pin |

### Buttons Not Working

| Symptom | Cause | Fix |
|---------|-------|-----|
| Always pressed | Pullup not enabled | Enable in firmware |
| Never registers | Bad solder joint | Reflow connection |
| Intermittent | Cold joint | Resolder with more heat |

### USB Not Detected

| Symptom | Cause | Fix |
|---------|-------|-----|
| No device | USB cable (data) | Try different cable |
| Unknown device | Driver issue | Install ESP32 drivers |
| Resets constantly | Power issue | Use powered USB hub |

---

## Maintenance

### Regular Care

- **Clean joystick** — Wipe with dry cloth
- **Check connections** — Ensure wires haven't loosened
- **Update firmware** — Check for updates via web UI

### If Something Breaks

- **Button caps** — Reprint from STL files
- **Joystick** — Replace module (~$3)
- **Enclosure** — Reprint damaged parts

---

## Safety Notes

1. **Unplug before opening** — Prevent electrical damage
2. **No liquids near device** — Electronics are not waterproof
3. **Secure mounting** — Prevent falls that could damage device
4. **Don't over-tighten** — Plastic threads can strip

---

## Next Steps

- [CONFIGURATION.md](CONFIGURATION.md) — Set up profiles and sensitivity
- [FIRMWARE.md](FIRMWARE.md) — Update firmware
- [README.md](../README.md) — Full project overview
]]>
