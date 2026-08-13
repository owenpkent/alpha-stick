# Modes and Layers

How one stick, moved by one hand that may only manage a few grams and a few millimetres,
reaches every control a game expects.

This document specifies the **layer** system: a data-driven way to rebind what stick
deflection *means* at runtime, selected by a single shift input. It sits inside the existing
mode system described in [FIRMWARE.md](FIRMWARE.md) and does not replace it.

Status: **specification only.** Nothing here is implemented yet. `as_modes/router.cpp` today
routes a fixed mode to the HID writers with no layer concept.

---

## Provenance and licensing

The idea that a single low-force joystick can drive a whole gamepad by shifting between
directional sub-modes is not ours. It is the core of the **J-Method**, built by Graham Law
(CelticMagic) and Barrie Ellis (OneSwitch) for the Titan Two, and it has been in real daily
use by disabled gamers for years. Alpha Stick owes it the credit.

**This specification is a clean-room design.** It was written from the general interaction
concepts (a shift input, directional selection, latching, per-user pruning of unused
sub-modes), not from their source. No GPC code, engine logic, mode table, or file structure
from the J-Method has been read into, copied into, translated into, or paraphrased into this
repository, and none may be.

Rules for anyone implementing this:

1. **Do not port the J-Engine.** GCM's `J-System settings` / `J-Game settings` / `J-Engine`
   files are Law and Ellis's copyrighted work, distributed to individual users for private
   use. Alpha Stick is public and MIT-licensed. Machine-translating those files to C++ is the
   same infringement as typing them out.
2. **Implement from this document.** If this spec is unclear, fix this spec. Do not go read
   theirs.
3. **Prior art you may reuse freely:** Owen Kent's own
   [Gaming-Scripts](https://github.com/owenpkent/Gaming-Scripts) and
   [Feather-with-sip-and-puff](https://github.com/owenpkent/Feather-with-sip-and-puff), both
   MIT under his own copyright. The tap-to-swap idiom in section 5.3 comes from there.

Interaction conventions are not copyrightable; implementations are. Keep the line clean.

---

## 1. Why layers exist

A gamepad asks for two sticks, a D-pad, four face buttons, four shoulders, and a menu cluster.
A user who can produce 3 gf over 4 mm of travel has one stick and, at best, a couple of switch
inputs.

The Feather solves this by shipping three separate firmware images (joystick, mouse, GCM) and
asking the user to reflash to change personality. The Titan Two solves it downstream, by
rewriting a dumb joystick's HID reports in an adapter.

Alpha Stick has an ESP32-S3 inside the stick, so it can solve it at the source: the same
physical deflection produces different output depending on which layer is active, switched in
firmware, with no adapter in the chain and no re-enumeration.

---

## 2. Modes vs layers

Two separate concepts. Do not conflate them.

| | **Mode** (exists today) | **Layer** (this spec) |
|---|---|---|
| Answers | Which HID personality do we present? | What does deflection do right now? |
| Values | `GAMEPAD`, `MOUSE`, `KEYBOARD`, `DUAL`, `ATOS` | user-defined set, 8 defaults below |
| Changes | Rarely, per profile or host | Constantly, mid-game, by shift gesture |
| Code | `Profile::mode`, `as_pipeline/profile.h` | `Profile::layers`, new |
| Effect on USB | None, descriptors are constant | None, descriptors are constant |

Layers are active in `GAMEPAD`, `MOUSE`, `KEYBOARD`, and `DUAL`. Layers are **hard-disabled in
`ATOS` mode** (see section 8).

---

## 3. What a layer is

A layer binds the stick's X/Y (and optionally Z) to outputs. Exactly two kinds:

### 3.1 `axis` layers

Deflection passes through as an analog pair. Full resolution, full curve, full filter.

| Field | Meaning |
|---|---|
| `target` | `left_stick`, `right_stick`, `mouse`, `scroll`, `triggers` |
| `scale` | 0.0 to 2.0, applied after the profile curve. Lets one layer be a slow precision variant of another |
| `invert_x`, `invert_y` | Per layer, on top of the profile-level mount inversion |

### 3.2 `buttons` layers

Deflection is quantized to a compass direction, and each direction fires a button. This is what
turns one stick into a D-pad, a face-button cluster, or a shoulder set.

| Field | Meaning |
|---|---|
| `dirs` | `4` (N/E/S/W) or `8` (adds diagonals) |
| `threshold` | 0.0 to 1.0 deflection at which the direction activates. Default 0.35 |
| `hysteresis` | Release happens at `threshold - hysteresis`. Default 0.10, prevents chatter |
| `exclusive` | `true` (default): only the dominant direction fires. `false`: allows two adjacent |
| `bind` | Map of direction to action, e.g. `{"N": "dpad_up", "E": "button_b"}` |
| `repeat_ms` | Optional auto-repeat while held, `0` disables. For menu navigation |

`threshold` and `hysteresis` are the accessibility-critical fields. A user with tremor needs a
high threshold and wide hysteresis; a user with tiny range needs both low.

### 3.3 Actions

Any `bind` value may be: `left_stick_up/down/left/right`, `right_stick_*`, `dpad_*`,
`button_a/b/x/y`, `l1/r1/l2/r2/l3/r3`, `start`, `select`, `home`, `capture`, `lmb`/`rmb`/`mmb`,
`scroll_up/down`, `key_<name>`, `layer:<name>` (switch to a named layer), or `none`.

`layer:<name>` is deliberate: it lets a layer act as a menu of other layers without consuming
the shift input.

---

## 4. Default layer set

Eight layers in bank 1, addressed by the eight compass directions. Every one is a data entry,
not code.

| Address | Name | Kind | What it does |
|---|---|---|---|
| `1W` | `left-stick` | axis | Passthrough to left stick. The home layer |
| `1E` | `right-stick` | axis | Passthrough to right stick, for camera |
| `1N` | `dpad` | buttons (4) | D-pad, with auto-repeat for menus |
| `1S` | `face` | buttons (4) | N/E/S/W to Y/B/A/X |
| `1NE` | `shoulders` | buttons (4) | N/S to L1/R1, E/W to L2/R2 |
| `1NW` | `mouse` | axis | Velocity pointer. The alt layer |
| `1SE` | `scroll` | axis | Y to wheel, X to horizontal scroll |
| `1SW` | `system` | buttons (4) | Start, Select, Home, Capture |

**All eight ship disabled except `left-stick` and `mouse`.** Layers are opt-in. A user who
wants two layers should not have to navigate past six they never use. See section 5.4.

Banks 2 to 4 are empty by default and exist for users who genuinely need more than eight.

---

## 5. Selecting a layer

### 5.1 Shift sources

The shift input is abstracted, never hardcoded to one physical control. `shift.source` is a
list, OR'd together, drawn from:

`zclick`, `jack1` through `jack4`, `sip_soft`, `sip_hard`, `puff_soft`, `puff_hard`,
`button_combo`, `none`.

`none` disables layer switching entirely without deleting the layer set, which is the right
behaviour for a shared or loaner device.

A control used as a shift source is **suppressed from normal output** while
`shift.suppress_source` is true (the default), so shifting never leaks a stray click into the
game.

### 5.2 Direct select (default)

1. Assert shift.
2. Push the stick toward a compass direction past `select_threshold` (default 0.60, higher
   than the layer threshold, so selection is deliberate).
3. Release shift.

The layer at that address becomes active. Stick output is **zeroed for the whole gesture** so
the selection movement never reaches the host.

Releasing shift without passing the threshold is a tap, see 5.3.

### 5.3 Tap to swap

A shift press shorter than `tap_ms` (default 400) with no directional push toggles between
`home` and `alt`. Two-layer users, who are the majority, then never need a directional gesture
at all: one tap flips stick to mouse and back.

This idiom is taken from Owen's own MIT-licensed `owen Joystick to 4 Modes.gpc`.

### 5.4 Banks (optional)

If `shift.banks` is greater than 1, the number of shift taps *before* the directional push
selects the bank. Two taps then push north selects `2N`. Users who never tap stay in bank 1
forever and never see this feature.

4 banks x 8 directions = 32 layers from a single switch. Almost nobody should turn this on.

### 5.5 Latching

`shift.latching: true` converts shift from hold-to-select into press-to-arm. Press once to
enter selection, push the stick, and selection commits on the directional push with no second
press.

This is not a preference, it is an access requirement. A user who cannot sustain a press cannot
use momentary shift at all.

### 5.6 Timeout

Selection state auto-aborts after `select_timeout_ms` (default 3000) and reverts to the
previous layer. A user who arms selection and then cannot complete the gesture must not be
stranded with a dead stick.

### 5.7 Enable mask

Every layer carries `enabled`. Disabled layers are skipped, not silently mapped to something
else: pushing toward a disabled address aborts the gesture and keeps the current layer, with
error feedback. Pruning the set to the two or three layers a person actually uses is the single
biggest usability win available here.

---

## 6. Feedback

The user must know which layer is active without looking away from the game. Hardware is two
WS2812B LEDs and an optional DRV2605L + LRA ([HARDWARE.md](HARDWARE.md)).

| Event | LED | Haptic |
|---|---|---|
| Layer active | Steady colour, per layer, brightness from profile | none |
| Shift armed | Both LEDs pulse slowly | short tick |
| Layer committed | One flash in the new layer's colour | one pulse |
| Disabled address | Two fast red flashes | double tick |
| Timeout abort | Fade back to previous colour | none |

Layer colour is a per-layer profile field. Brightness must be configurable down to off:
some users are light-sensitive, and some builds sit close to the face on a chin boom.

---

## 7. Switching rules

Non-negotiable, all of them:

1. **Release everything.** On layer change, every button the outgoing layer asserted is
   released in the same report. A layer switch must never leave a button stuck down.
2. **No re-enumeration.** Layers change which values fill the reports, never the descriptors.
3. **Zero during selection.** Stick output is zero from shift assert to gesture end.
4. **One tick.** Layer evaluation runs inside the existing 1 kHz router task. It must not add a
   frame of latency. R7 (<5 ms stimulus to report) still applies with layers active.
5. **Persist the active layer** across a mode change, but reset to `home` on power-up. Waking
   up in an unexpected layer is disorienting.

---

## 8. ATOS and the DRIVE build

Layers are **compiled out** when `AS_BUILD_PROFILE=DRIVE`, and disabled at runtime in `ATOS`
mode regardless of build.

Drive intent must mean exactly one thing. A wheelchair input where a stray tap can silently
remap "forward" is not an input device, it is a hazard. This matches the existing split in
[FIRMWARE.md](FIRMWARE.md): same hardware, separate build, different integrity domain.

---

## 9. Profile schema

Extends the JSON in [FIRMWARE.md](FIRMWARE.md) and [CONFIGURATION.md](CONFIGURATION.md).

```json
{
  "name": "default",
  "mode": "gamepad",

  "layers": {
    "enabled": false,
    "home": "left-stick",
    "alt": "mouse",

    "shift": {
      "source": ["zclick"],
      "suppress_source": true,
      "latching": false,
      "tap_ms": 400,
      "select_threshold": 0.60,
      "select_timeout_ms": 3000,
      "banks": 1
    },

    "set": [
      { "at": "1W", "name": "left-stick", "enabled": true,
        "kind": "axis", "target": "left_stick", "scale": 1.0, "led": "#2060ff" },

      { "at": "1NW", "name": "mouse", "enabled": true,
        "kind": "axis", "target": "mouse", "scale": 1.0, "led": "#ffffff" },

      { "at": "1N", "name": "dpad", "enabled": false,
        "kind": "buttons", "dirs": 4, "threshold": 0.35, "hysteresis": 0.10,
        "exclusive": true, "repeat_ms": 250, "led": "#20c020",
        "bind": { "N": "dpad_up", "S": "dpad_down",
                  "W": "dpad_left", "E": "dpad_right" } },

      { "at": "1S", "name": "face", "enabled": false,
        "kind": "buttons", "dirs": 4, "threshold": 0.45, "hysteresis": 0.12,
        "exclusive": true, "repeat_ms": 0, "led": "#ff8000",
        "bind": { "N": "button_y", "E": "button_b",
                  "S": "button_a", "W": "button_x" } }
    ]
  }
}
```

Omitted addresses are absent, not disabled: the set is sparse and only carries what the user
configured.

---

## 10. Firmware shape

New component `as_modes/layers.{h,cpp}`, consumed by the existing router task. Sketch, not
final:

```c++
enum class LayerKind : uint8_t { AXIS = 0, BUTTONS };
enum class AxisTarget : uint8_t { LEFT_STICK = 0, RIGHT_STICK, MOUSE, SCROLL, TRIGGERS };

struct Layer {
    char     name[16];
    uint8_t  addr;          // bank<<4 | direction index (0=N, clockwise)
    bool     enabled;
    LayerKind kind;
    uint32_t led_rgb;

    AxisTarget target;      // AXIS
    float      scale;

    uint8_t  dirs;          // BUTTONS: 4 or 8
    float    threshold, hysteresis;
    bool     exclusive;
    uint16_t repeat_ms;
    uint8_t  bind[8];       // action ids, index matches direction
};

struct LayerConfig {
    bool     enabled;
    uint8_t  home, alt;     // indices into set[]
    uint8_t  shift_sources; // bitmask
    bool     suppress_source, latching;
    uint16_t tap_ms, select_timeout_ms;
    float    select_threshold;
    uint8_t  banks, count;
    Layer    set[16];
};
```

`Profile` gains a `LayerConfig`. That is a large struct change, so bump `CONFIG_BLOB_VERSION`
in `as_config` and write the NVS migration before merging, or every existing profile is lost on
update.

The router grows a small state machine (`IDLE -> ARMED -> SELECTING -> COMMIT`) evaluated once
per 1 ms tick before the current mode routing.

---

## 11. Phasing

| Phase | Scope |
|---|---|
| **L0** | Two layers (`home`/`alt`), tap-to-swap only, no directional gesture, no banks. Covers most users and validates the release-everything rule |
| **L1** | Direct select over bank 1, `buttons` layers, threshold and hysteresis, enable mask, LED feedback |
| **L2** | Latching, timeout, `layer:<name>` actions, configurator UI |
| **L3** | Banks, haptic feedback, per-game profile presets |

L0 should land alongside the jack and sip/puff input subsystem, since the shift source
abstraction is not useful until there is more than one thing to bind it to.

---

## 12. Open questions

1. Is `zclick` a viable default shift source, or does a press-to-click on a 3 gf flexure fire
   too easily during normal aiming? Depends on PRD open question 2.
2. Does the 8-direction gesture survive a real tremor profile, or does the default set need to
   drop to 4 directions?
3. Should layer state be per-profile or global? Per-profile is more flexible; global is easier
   to reason about when the user switches profiles mid-session.
4. How does a layer interact with `DUAL` mode, where gamepad and mouse already stream at once?
