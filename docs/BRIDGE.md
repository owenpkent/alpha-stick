# AS-Bridge: replacing the adapter stack

A ~$4 board that gives Alpha Stick console reach without us writing a single line of console
protocol code.

Companion to [TOPOLOGY.md](TOPOLOGY.md) (how several sticks become one device) and
[MODES.md](MODES.md) (what deflection means). This document is about the last hop, from our
device to a console.

Status: **specification only.** Nothing here is built. The add-on described in section 5 does
not exist.

---

## 1. The strategy in one line

**We write an input driver. We inherit everything hard.**

[GP2040-CE](https://gp2040-ce.info/) is maintained, open, MIT-licensed gamepad firmware for the
RP2040 that already speaks PC, PS3, PS4, PS5, Switch, Xbox One, Steam Deck, MiSTer, and Android
across 14 input modes, including the authentication passthrough that consoles require.

It takes input from GPIO buttons and from analog joysticks wired to ADC pins. It does not know
how to listen to an Alpha Stick. That gap, one input add-on, is the entire piece of work.

Everything we would otherwise have to build and maintain forever (console identity, descriptor
quirks per platform, auth relay timing, per-console compatibility regressions) stays upstream
with the people who already do it well.

### Why not fork

MIT on both sides means forking is legally trivial, which is exactly why it is tempting and
wrong. A fork inherits the maintenance burden of every console the upstream supports. An
upstream input add-on inherits none of it.

Contribute the add-on. If upstream declines it, vendor GP2040-CE unmodified and carry the
add-on as a patch, in that order of preference.

### The DRM question, settled

An earlier discussion flagged that shipping console auth-relay code in an accessibility repo is
a bad idea. This approach means **we ship none of it.** The auth passthrough lives upstream in a
widely used project. We contribute a driver that reads a serial protocol. That is the whole
difference, and it is worth preserving deliberately: no auth, crypto, or console-handshake code
belongs in this repository.

---

## 2. Reference configuration

Two sticks and two switches, which is the configuration this design is sized for:

```
  Alpha Stick A ──┐
  (left stick)    │
                  ├── AS-Link ──> AS-Bridge ──USB──> console
  Alpha Stick B ──┘                  │
  (right stick)                      └──USB-A──> auth device (per platform, section 6)

  switch ──3.5mm──> Stick A jack 1
  switch ──3.5mm──> Stick A jack 2
```

Switches plug into the **stick**, not the bridge. Switches belong within reach of the user's
body; the bridge sits next to the console. Their state rides in the AS-Link frame's `buttons`
field. The bridge also exposes GPIO for buttons wired locally, which GP2040-CE handles natively
at no cost to us.

GP2040-CE's analog model supports exactly two sticks, each assignable to left or right, with
stick 2 forced to the opposite role from stick 1. Two sticks is the native case, not a stretch.

> **Note on the ADC limit.** GP2040-CE's Analog add-on docs state that a stock Pico breaks out
> only three ADC pins, so a physical-potentiometer build gets one stick, not two. **This does
> not apply to us.** Our add-on injects axis values digitally and never touches the ADC. The
> two-stick limit is a wiring constraint we bypass entirely.

---

## 3. Choose RP2040, not RP2350

Use a **Raspberry Pi Pico** or equivalent RP2040 board.

RP2350 / Pico 2 is tempting on paper, and an earlier draft of this project's notes recommended
it. GP2040-CE is RP2040-centric, its own notes describe RP2350 support as very early and
quirky, and its published performance figures are measured on RP2040. The maturity is on the
older chip. Revisit no earlier than 2027.

---

## 4. Board and cabling

### What plugs into what

Only **two** USB connections exist in this design. The sticks are not among them.

```
  console USB  <──USB-C/micro──  [ AS-Bridge ]  ──USB-A──>  auth device
   (device port,                   │       │                (section 6)
    powers the bridge)             │       │
                         4-wire UART       4-wire UART or ESP-NOW
                                   │       │
                              Stick A     Stick B
                               │     │
                     switch ───┘     └─── switch    (3.5 mm jacks on the stick)
```

| Connection | Type | Carries |
|---|---|---|
| Bridge to console | Native USB device port | The emulated controller; also powers the bridge |
| Bridge to auth device | USB-A host port (PIO-USB) | Authentication handshake only |
| Stick to bridge | 4-wire UART: 5 V, GND, TX, RX | AS-Link frames |
| Switch to stick | 3.5 mm jack | Contact closure, arrives as `buttons` bits |

### The host port is not free

A bare Pico has **one** USB connector, and it is the device port. The USB-A host socket has to
be added. GP2040-CE's requirements for it:

- **D+ on any GPIO, D- on the immediately adjacent pin** (X+1 or X-1).
- A 5 V source (VBUS) and ground.
- A separate GPIO for **Enable 5V**, which gates power to the host port.
- Configured in the web configurator's Peripheral Mapping page.

Upstream publishes a USB Host Port Installation guide and an official RP2040 Advanced Breakout
Board with a passthrough variant, wired with 2-pin JST 2.0 mm cables. Prefer their board over
inventing one.

### Board options

| Option | Cost | Trade |
|---|---|---|
| **Pico + upstream passthrough breakout** | ~$4 + breakout | Follows the documented path exactly. Two boards |
| **Adafruit Feather RP2040 with USB Type A Host (#5723)** | ~$15 | Both ports on one board. D+ GPIO16, D- GPIO17, 5 V enable GPIO18, which fits the rules above. **Not on GP2040-CE's board list**, see open question 6 |
| **Roll our own** | PCB spin | Only worth it at v2, below |

### v2, wireless

Add an ESP32-S3 as an ESP-NOW receiver, UART to the RP2040. Each chip does what it is good at:
the S3 owns the radio and AS-Link, the RP2040 owns console USB.

~$12 in parts. The S3 half is the ESP-NOW dongle already carried in the BOM at +$9, so v2 is
largely a merge of a board already planned rather than a new one.

### Do not put a USB connector on the stick cable

5 V, GND, TX, RX is physically what a USB cable carries. A USB-shaped connector on a UART link
means that eventually somebody plugs it into a real USB port and drives 5 V from a host onto a
UART pin.

**Use a 4-pin JST-SH.** It costs nothing and it is not physically capable of the mistake.

### Power

The bridge is bus-powered from the console. The auth device hangs off the host port and draws
from the same VBUS, and the sticks may draw their 5 V from the bridge as well.

**Budget this before laying out a board.** A DualSense is not a low-current device, and a chain
of console to bridge to controller to two sticks is the obvious place for this design to fall
over. Measure before committing to a bus-powered v1; a powered hub or an injected 5 V rail may
prove mandatory. The Enable 5V GPIO at least gives firmware control over when the host port
draws at all.

---

## 5. The AS-Link input add-on

The only code we write.

### Contract

| | |
|---|---|
| Input | COBS-framed `aslink_input_v1_t` on UART, plus 1 Hz `aslink_health_v1_t` |
| Output | `GamepadState` fields, below |
| Rate | Frames arrive at up to 1 kHz; GP2040-CE polls at 1 kHz. One frame per poll, newest wins |
| Buffering | None. Read the newest complete frame, discard backlog. Never queue |

### The interface we implement

`GPAddon`, verified from `headers/gpaddon.h`:

```c++
virtual bool available();
virtual void setup();
virtual void preprocess();
virtual void process();
virtual void postprocess(bool);
virtual std::string name();
virtual void reinit();              // only if pin assignments can change
virtual USBListener * getListener();  // only for add-ons needing USB host
```

We do not implement `getListener()`. Our input arrives on UART, not USB, so the add-on never
touches the host port and cannot contend with the PS passthrough add-on for it.

### The target state

Verified from `headers/gamepad/GamepadState.h`:

```c++
struct GamepadState {
    uint8_t  dpad {0};
    uint8_t  dpadOriginal {0};
    uint32_t buttons {0};
    uint16_t aux {0};
    uint16_t lx, ly, rx, ry {GAMEPAD_JOYSTICK_MID};
    uint8_t  lt {0}, rt {0};
    float    ema_1_x, ema_1_y, ema_2_x, ema_2_y;
};

#define GAMEPAD_JOYSTICK_MIN 0
#define GAMEPAD_JOYSTICK_MID 0x7FFF   // 32767
#define GAMEPAD_JOYSTICK_MAX 0xFFFF
```

Three things this changes versus a naive port:

1. **`dpad` is a separate field, not part of `buttons`.** Any layer binding that produces a
   D-pad direction writes `state.dpad`, not the button mask. Getting this wrong produces a
   device where the D-pad silently does nothing.
2. **`buttons` is `uint32_t`.** Our AS-Link `buttons` is 16-bit, so it maps into a wider space
   with room to spare. The mapping table's output type is 32-bit.
3. **`lt` / `rt` are `uint8_t` analog triggers**, with a `hasAnalogTriggers` flag. A trigger
   layer has a real analog path rather than an on/off approximation.

Leave `ema_*` and `dpadOriginal` alone. Those belong to GP2040-CE's own processing.

### Per frame

1. **Validate.** Check `ver`, then `crc32`. Drop silently on failure; a corrupt frame is not an
   input event.
2. **Identify.** Read `src_id` (see section 7). Look up which stick role it is bound to.
3. **De-dupe.** Compare `seq` against the last seen value **for that source**. Drop duplicates
   and regressions. Per-source, never global: two sources have independent counters and a shared
   check would discard valid frames constantly.
4. **Convert axes.** `x_q15` is Q15 signed (-32768 to +32767, meaning -1.0 to +1.0). The target
   is `uint16_t` spanning 0 to 65535. A plain `q15 + 32767` **underflows at full negative
   deflection** (-32768 + 32767 = -1, which wraps to 65535 on a uint16, sending hard left as
   hard right). Offset by 32768 and clamp:

   ```c++
   static inline uint16_t q15_to_gp(int16_t q) {
       if (q == 0) return GAMEPAD_JOYSTICK_MID;      // exact centre
       int32_t v = (int32_t)q + 32768;               // 0 .. 65535
       return (uint16_t)(v > GAMEPAD_JOYSTICK_MAX ? GAMEPAD_JOYSTICK_MAX : v);
   }
   ```

   The centre special-case exists because `GAMEPAD_JOYSTICK_MID` is `0x7FFF` (32767), one below
   the 32768 the offset produces. One LSB in 65536 is inaudible, but resting exactly on the
   firmware's own centre constant costs nothing and avoids a permanent 1-LSB bias.

5. **Map buttons.** `buttons` in the frame is a 16-bit field of jacks, sip/puff thresholds, and
   Z-click. A configurable table maps each bit to a `GamepadState` button (`uint32_t`) **or a
   `dpad` flag**, since those are separate fields. Defaults should mirror the profile button
   mapping in [CONFIGURATION.md](CONFIGURATION.md) so the same switch does the same thing whether
   the stick is on a PC or behind a bridge.
6. **Enforce cardinal exclusivity ourselves.** GP2040-CE runs SOCD cleaning **before** add-ons,
   so anything we write afterwards is not cleaned. Its own docs state that add-ons must handle
   opposing simultaneous cardinal directions themselves. If a layer binds directions to `dpad`,
   the add-on must guarantee that left and right, or up and down, are never asserted together.
   The `exclusive` flag on `buttons` layers in [MODES.md](MODES.md) already does this on the
   stick side; the add-on must not assume it and should enforce again.
7. **Refresh the TTL timer** for that source.

### On timeout

When a source's `ttl_ms` expires, or its 1 Hz health frames stop:

- That source's axes centre.
- That source's buttons release.
- **Every other source keeps working.**

The health-frame rule already exists in the AS-Link contract and carries over unchanged: a
source whose health frames stop is dead regardless of input frames still arriving.

This is not a nicety. Without it, a stick that loses power mid-game leaves its last axis value
latched at whatever deflection it happened to be at.

### Configuration

Expose through GP2040-CE's existing web configurator rather than inventing a second config
channel: source-to-stick binding, the button map, and the TTL timeout.

### What the add-on must not do

No curve, no deadzone, no filtering, no layer logic. All of that already ran on the stick, where
it belongs, tuned to the hand using it. The add-on is a transport, and doing any of it twice
produces a device that feels wrong in ways that are miserable to debug.

GP2040-CE's own deadzone and auto-calibration options should be set to pass-through for our
sources for the same reason.

---

## 6. Per-platform authentication

What the bridge needs plugged into its USB-A host port.

| Platform | Auth device | Notes |
|---|---|---|
| PC, Mac, Linux | **None** | Also true without a bridge; the stick plugs in directly |
| Android, iOS | **None** | BLE HID from the stick, no bridge needed |
| Steam Deck, MiSTer | **None** | |
| Switch | **None** | Or skip the bridge entirely via the stick's BLE Pro Controller path |
| PS4 | A DualShock 4 you own | Console re-verifies periodically, so it stays plugged in |
| PS5 | A DualSense you own | Reached through GP2040-CE's PS5 mode |
| Xbox One, Series | **Mayflash MagicBoots or Magic-X** | Licensed Xbox controllers do **not** work as the auth source here. See below |

### Xbox needs verifying before anyone spends money

GP2040-CE's Xbox One mode explicitly does not accept an Xbox Wireless Controller, Elite
controller, or other licensed first-party pad as its auth device. It needs a MagicBoots or
Magic-X converter.

**Whether those converters still function after Microsoft's November 2023 block on unauthorized
third-party accessories is unverified.** Confirm current behaviour before recommending this path
to a user, and do not put an Xbox claim in the README until someone has done it on real
hardware.

We never touch XSM3 either way. We plug in a device that already holds it.

---

## 7. Dependency on the `src_id` change

This design **requires** the wire-format change in [TOPOLOGY.md](TOPOLOGY.md) section 6:
`aslink_input_v1_t`'s `uint8_t reserved` becomes `uint8_t src_id`.

With two sticks on one UART link there is no peer MAC to fall back on, so without a source ID
the bridge cannot tell left from right. The two-stick reference configuration in section 2 does
not work until this lands.

Frame stays 20 bytes, `ASLINK_VERSION` does not change, `0` keeps its current single-source
meaning.

---

## 8. Against the Titan Two

| | Titan Two | AS-Bridge |
|---|---|---|
| Cost | ~$100 | ~$4 (v1), ~$12 (v2) |
| Latency | ~1 ms | 0.76 ms XInput, 0.91 ms PS5 (vendor-reported, RP2040) |
| Polling | 1 kHz | 1 kHz default |
| Scripting | GPC, a real language | Layers on the stick, plus a macro engine we still owe |
| Third-party controller support | Large maintained database | None, and none needed when the input is our own stick |
| Console coverage | Broad, commercial support | 14 modes, community support |
| Source | Closed | MIT |

The honest gaps are scripting and support, not capability. GPC is Turing-complete and the layer
system is data; timed macros like a combo with a `wait(500)` have no home yet, on either side of
the link. That work is owed regardless of which adapter is in the chain.

---

## 9. Phasing

| Phase | Scope |
|---|---|
| **B0** | `src_id` wire change (blocks everything, see section 7) |
| **B1** | Add-on against a bench Pico, one stick over UART, PC / XInput only. Proves the transport |
| **B2** | Two sticks, button mapping, TTL timeout, web configurator fields |
| **B3** | Upstream the add-on to GP2040-CE. Vendor with a patch only if declined |
| **B4** | Auth passthrough verified per platform on real hardware, starting with PS since Xbox is uncertain |
| **B5** | v2 wireless board |

B1 is small: read a UART, validate a CRC, fill a struct. Most of the risk in this document is in
B4, where the answers depend on other people's hardware and Microsoft's current policy.

---

## 10. Open questions

1. Does the Analog add-on need disabling to avoid contention when our add-on writes `lx/ly/rx/ry`
   directly? The state fields are plainly writable and our add-on registers no USB listener, so
   the risk is two add-ons writing the same field in the same tick rather than anything
   architectural. Confirm add-on execution order before assuming last-write-wins.
2. ~~Does PS passthrough already own the USB host port?~~ **Resolved.** Host access goes through
   the optional `getListener()` on `GPAddon`. Our add-on reads UART and implements no listener,
   so it cannot contend for the host port at all.
3. Is a bus-powered bridge viable with a DualSense attached, or is injected 5 V mandatory?
   Section 4 flags this as the most likely hardware failure.
4. Do the MagicBoots / Magic-X converters still work post-November 2023?
5. Should the bridge expose its own 3.5 mm jacks after all? Section 2 argues switches belong on
   the stick, but a user with a fixed console-side mount may disagree, and jacks are cheap.
6. Does GP2040-CE run on the Adafruit Feather RP2040 with USB Type A Host (#5723)? Its pinout
   fits the rules in section 4, but it is not on the official board list, and Adafruit notes
   PIO-USB host consumes the second core and both PIO blocks. Confirm before buying, or take the
   Pico plus upstream breakout instead.
