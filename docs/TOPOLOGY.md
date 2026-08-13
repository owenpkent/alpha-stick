# Topology: hub, satellites, and what replaces the adapter stack

How several Alpha Sticks, switches, and other inputs combine into **one** device the host sees,
without an Xbox Adaptive Controller or a Titan Two in the middle.

Companion to [MODES.md](MODES.md) (what deflection *means*) and [FIRMWARE.md](FIRMWARE.md)
(how a single stick is built). This document is about how *many* inputs become one.

Status: **specification only.** `AS_ROLE_HUB` does not exist yet. Today's firmware ships
`AS_ROLE_STICK` and `AS_ROLE_DONGLE` ([firmware/main/Kconfig.projbuild](../firmware/main/Kconfig.projbuild)).

---

## 1. The problem, stated from a real setup

A working multi-input rig today looks like this:

```
  Feather joystick ──USB──┐
  Feather joystick ──USB──┤
  switch ──────────3.5mm──┼──> XAC ──USB──> Titan Two ──USB──> console
  switch ──────────3.5mm──┘
```

Four devices, three hops, roughly $400, and the mode logic lives two boxes downstream of the
hand that moves the stick.

**Two of those three boxes exist only because the joysticks are dumb HID devices.** They cannot
merge, cannot remap, and cannot talk to each other, so an aggregator and a scripting adapter
have to be bought and stacked to compensate.

Alpha Stick has an ESP32-S3 in the stick. It can do the merging and the remapping at the
source.

---

## 2. Where each job goes

### Xbox Adaptive Controller

| Job | Where it goes |
|---|---|
| USB host for two joysticks | **Gone.** Sticks talk AS-Link, not USB |
| 3.5 mm switch jacks | **Onto our board.** 4x PJ-320A, [HARDWARE.md](HARDWARE.md) parts table |
| Merge into one controller | **`AS_ROLE_HUB`**, section 4 |
| Xbox-licensed output | **Cannot move.** The only irreplaceable part |

### Titan Two

| Job | Where it goes |
|---|---|
| Macro and mode scripting | **`as_modes`** plus the layer system in [MODES.md](MODES.md) |
| Slot switching | **NVS profiles**, already built |
| Multi-platform output protocol | Composite HID covers PC and mobile; consoles need an **AS-Bridge** ([BRIDGE.md](BRIDGE.md)) |
| Console authentication | **Never ours.** Relayed from a licensed device by upstream firmware, see [BRIDGE.md](BRIDGE.md) section 6 |

The result is platform-dependent and we should say so plainly rather than claim a clean sweep:

| Platform | Direct from the stick | Via XAC | Via AS-Bridge |
|---|---|---|---|
| PC, Linux, macOS | **Yes**, nothing else needed | n/a | n/a |
| Android, iOS | **Yes**, BLE HID | n/a | n/a |
| Switch | BLE Pro-Controller path | n/a | Yes |
| Xbox | No | Yes, but 8 buttons only (2.1), keep the Titan Two | Yes, full control set, auth dongle unverified |
| PlayStation | No | Access controller AUX ports, switch signals only | Yes, with a controller you own |

On PC the stick alone removes both boxes. On consoles the choice is between the XAC's narrow
path and a bridge; the Titan Two is only unavoidable on the XAC path.

### 2.1 The Xbox bottleneck: why the layer engine cannot move upstream

An earlier draft of this document claimed the Titan Two becomes redundant on Xbox. **That was
wrong**, and the reason is structural rather than a detail to be engineered around.

In the adapter stack, the layer engine sits in the Titan Two, which is **downstream** of the
XAC:

```
  stick ──axes only──> XAC ──controller──> Titan Two ──everything──> console
                                            ▲
                                    layer engine lives here,
                                    after the XAC's bottleneck
```

The stick only ever had to send axes. The XAC turned axes into a controller. The Titan Two then
expanded that into the D-pad, face buttons, triggers, and menu cluster. Nothing was constrained,
because the expansion happened *after* the XAC.

Moving the layer engine into the stick puts it **upstream** of the XAC, so everything it
produces must now fit through the XAC's USB host port. That port is narrow:

| XAC USB host port constraint | Source |
|---|---|
| Device must expose **only** a joystick. No keyboard, mouse, or media interfaces | adafruit/circuitpython [#1696](https://github.com/adafruit/circuitpython/issues/1696), [mo-vis manual](https://user-manuals.mo-vis.com/en-US/HID_joystick_user_manual/p025_61_xbox_xac.html) |
| **8 buttons maximum**, fixed mapping, different per port, not user-remappable | [darthcloud teardown](https://hackaday.io/project/170365/log/179869-xbox-one-adaptive-controller) |
| Axis range **0 to 255 unsigned**; Logical Min/Max removed from the descriptor | circuitpython #1696 |
| Left port replaces the left thumbstick, right port the right thumbstick | [Xbox Support](https://support.xbox.com/en-US/help/account-profile/accessibility/xac-user-guide-using-external-joysticks) |
| ~100 mA per host port unless the XAC has its own 5 V supply | reported alongside the above; **verify on the bench** |
| Compatibility is empirically narrow (a Logitech Extreme 3D worked; other USB gamepads did nothing) | darthcloud teardown |

Eight fixed-mapped buttons cannot carry a layer system whose entire purpose is reaching the
menu cluster and the shoulder set. **Through an XAC, keep the Titan Two.** Alpha Stick replaces
the *Feather* on that path, not the adapter stack.

This does not weaken the case on PC, where both boxes genuinely disappear.

The other way out is to not go through the XAC at all: an AS-Bridge reaches Xbox directly and
keeps the full control set, at the cost of an auth dongle whose current status is unverified.
See [BRIDGE.md](BRIDGE.md).

### 2.2 The XAC build profile

Alpha Stick's normal USB identity is a composite device: HID gamepad, mouse, and keyboard report
IDs plus a CDC config channel ([FIRMWARE.md](FIRMWARE.md)). Against the table above, that is
close to a list of everything the XAC rejects.

So plugging into an XAC needs a **second USB personality**, not a runtime mode:

| | Normal build | XAC build |
|---|---|---|
| Interfaces | HID (gamepad + mouse + keyboard) + CDC | **HID only, one joystick** |
| HID usage | Gamepad (0x05) | **Joystick (0x04)** |
| Axes | X/Y int16, Z uint8 | **X/Y/Z uint8, 0 to 255** |
| Logical Min/Max | Declared | **Omitted** (works around an XAC bug) |
| Buttons | 16 | **8** |
| Config channel | CDC | **None.** Configure over BLE or WiFi instead |
| Layers | Full action vocabulary | Restricted to the 8 buttons the XAC exposes |

**This cannot be a mode switch.** USB descriptors are fixed at enumeration, and the whole design
rests on never re-enumerating ([FIRMWARE.md](FIRMWARE.md)). The XAC personality is selected at
**boot** (a held button or a stored profile flag applied before TinyUSB starts) or at **build**
time via Kconfig. Boot-time is friendlier and costs one flag read before USB init.

Losing CDC is the sharp edge: the primary config path today is the CDC JSON protocol driven by
[tools/](../tools/). An XAC build has to be configured over BLE or the WiFi web UI, or by
plugging into a PC in the normal personality first. Say this in the user docs, because
"my configurator stopped seeing the stick" is otherwise a baffling failure.

Every constraint in 2.1 and 2.2 is **unverified against real hardware.** It comes from teardowns
and other people's bug reports. Confirm on a bench XAC in Phase 0 before any of it is promised
to a user.

---

## 3. Target topology

```
  Stick B (satellite) ───ESP-NOW/UART───┐
                                        ▼
  switch ──3.5mm──┐                Stick A (hub) ──USB/BLE──> host
  switch ──3.5mm──┴────────────────────┘
  sip/puff ────────────────────────────┘
```

Two devices, one hop. Stick A carries the cable, the jacks, and the layer engine. Stick B is
just another input source.

**Stick A becomes the left stick and Stick B the right stick natively.** The two-stick case is
plain axis assignment, not a layer trick, and layers remain available on top for reaching the
D-pad, face buttons, and menu cluster.

### Where the hub physically lives

| Option | Extra hardware | Trade |
|---|---|---|
| **A. Hub inside a stick** (recommended first) | None | Cable and jacks attach to whichever stick is nearest the host, which may not be where the user wants them |
| **B. Dedicated hub board** | One new PCB | Clean cabling for a wheelchair or desk mount, all jacks in one place, both sticks wireless |
| **C. Existing ESP-NOW dongle** | The dongle already in the BOM | Wireless everything, but the dongle has no jacks, so switches must plug into a stick |
| **D. AS-Bridge** | RP2040 board, ~$4 | The hub *is* the bridge. Only option that reaches a console, and the merge happens where GP2040-CE already expects two sticks. See [BRIDGE.md](BRIDGE.md) |

Start with A. It needs no new hardware and proves the merge logic. B follows if real users find
the cabling awkward, which they probably will on a chair mount.

---

## 4. Roles

Extends the existing `AS_ROLE` choice in Kconfig.

| Role | Sensor | Jacks | AS-Link | Layer engine | HID out |
|---|---|---|---|---|---|
| `STICK` (exists) | own | own | out | yes | yes |
| `SATELLITE` (new) | own | own | out only | **no** | no |
| `HUB` (new) | own | own | in, from N sources | yes, over merged state | yes |
| `DONGLE` (exists) | none | none | in | yes | yes |

`SATELLITE` is deliberately dumb: it runs the sensing pipeline (calibration, deadzone, curve,
filter are all local to the physical stick and its user's needs) and then streams. It does
**not** run layers. One brain in the system, in the hub.

`STICK` remains a complete standalone device. Nothing here makes a single-stick build more
complicated.

---

## 5. Merge semantics

This is the actual design work. A hub that merges naively will produce a device that fails in
ways that are hard to debug and dangerous to rely on.

### 5.1 Source binding is deterministic, never first-come

Each source binds to a role in the profile by its **source ID**, not by connection order:

```json
"sources": [
  { "id": 1, "bind": "left_stick",  "label": "left hand" },
  { "id": 2, "bind": "right_stick", "label": "chin" },
  { "id": 0, "bind": "left_stick",  "label": "hub's own sensor" }
]
```

A user must never power on their sticks in a particular order to get their controls back.

### 5.2 Per-source TTL decay

`aslink_input_v1_t` already carries `ttl_ms`. The hub tracks it **per source**. When one
satellite goes stale (battery dies, walks out of range, crashes), that source's axes decay to
zero and its buttons release. **Every other source keeps working.**

The failure mode this prevents: one stick dying and taking the whole device down mid-game, or
worse, leaving its last axis value latched at full deflection.

### 5.3 Per-source sequence tracking

`seq` is per-source and monotonic. The hub keeps a separate last-seen `seq` per source. A
single global de-dupe would drop valid frames the moment a second source exists, since two
independent counters will collide constantly.

### 5.4 Buttons OR together

Button bits from all sources OR into one 16-bit set, debounced after the merge, not before. Two
sources asserting the same button is one press, not chatter.

### 5.5 Axis conflict

Two sources bound to the same axis: **largest magnitude wins**, not sum. Summing lets two
half-deflections produce a full-deflection output the user did not intend, and it makes a stuck
source override a live one. Largest-magnitude also degrades sanely when one source decays to
zero under 5.2.

### 5.6 One layer state

Layer state lives in the hub and applies to all sources. Shift can be asserted from any source
(a jack on the hub, Z-click on a satellite, a puff). Selection uses whichever source is bound
to `layers.select_source`, defaulting to the first stick source.

Per-source layer state was considered and rejected: a user who cannot remember which of two
sticks is in which mode has been given a worse device, not a better one.

### 5.7 Health frames gate trust

A source whose 1 Hz health frames stop is treated as dead, regardless of input frames still
arriving. This rule already exists in the AS-Link contract and carries over unchanged.

---

## 6. Wire format change required

**`aslink_input_v1_t` has no source ID.** With more than one satellite the hub cannot tell
frames apart on UART, where there is no peer MAC to fall back on.

The fix is free today and expensive later, because
[aslink_frame.h](../firmware/components/as_aslink/include/aslink_frame.h) is vendored unchanged
into the ATOS gateway:

```c
    uint8_t flags;      // aslink_flags
    uint8_t src_id;     // was: reserved. 0 = unspecified (single-source legacy)
    uint32_t crc32;
```

Frame stays 20 bytes. `ASLINK_VERSION` does not need to change, because `0` retains the current
meaning for any existing single-source sender.

Do this before anything else in this document.

The health frame's `uint16_t reserved` should take a matching `src_id` in its low byte for the
same reason.

---

## 7. Pairing

Reuses the existing ESP-NOW pairing flow in [CONFIGURATION.md](CONFIGURATION.md) rather than
inventing a second one.

1. Hub: hold B1 + B2 for 5 s, LEDs pulse blue, hub enters pair-listen for 30 s.
2. Satellite: hold B1 + B2 for 5 s. It broadcasts its MAC and requested `src_id`.
3. Hub assigns the lowest free `src_id`, stores the MAC, both LEDs flash green.
4. Binding persists in NVS. Repeat for each satellite.

Wired satellites over UART skip pairing entirely and take `src_id` from their own profile.

`src_id` must be reassignable from the configurator without re-pairing, so a user can swap which
stick drives which axis without touching hardware.

---

## 8. Latency

| Path | Estimate |
|---|---|
| Old chain: Feather to XAC to T2 to console | 3 USB hops, each one poll interval |
| Hub's own sensor to host, wired | ~2-3 ms (R7 budget, unchanged) |
| Satellite to hub over ESP-NOW to host | ~3-5 ms |
| Satellite to hub over UART to host | ~2-3 ms |

Merging costs one 1 ms router tick at most: the hub reads the newest frame per source from its
mailbox and merges in place. It must not add a buffering stage. R7 applies to the hub's own
sensor path with no allowance for merge overhead.

---

## 9. What this does not give you

Stated plainly so nobody is surprised on the bench.

- **Console output of any kind.** A hub is still a USB HID device. Consoles need the AS-Bridge
  track in [BRIDGE.md](BRIDGE.md), which is separate work.
- **The full control set through an XAC.** That path gives the layer engine 8 fixed-mapped
  buttons and nothing else, so the Titan Two stays in that chain. Section 2.1.
- **Timed macros.** The Titan Two runs a real language, including combos with waits. Profiles
  and layers are data. A macro engine with timed sequences is a separate piece of work, already
  flagged as pending in [CONFIGURATION.md](CONFIGURATION.md).
- **Hosting third-party USB devices.** The ESP32-S3 has one USB OTG PHY, so it is host or
  device, never both. A hub cannot read existing USB joysticks and present as a gamepad at the
  same time. Anyone wanting that needs a second USB host controller, which is a hardware
  decision this document does not make.
- **Rumble, initially.** Though the DRV2605L and LRA are already in the BOM, so relaying host
  rumble output reports to the stick the user is holding is a genuinely better answer than the
  adapter's, once implemented.

---

## 10. Phasing

| Phase | Scope |
|---|---|
| **T0** | `src_id` wire change and per-source seq. Nothing else depends on anything else here |
| **T1** | `AS_ROLE_HUB` + `AS_ROLE_SATELLITE`, one satellite over UART, deterministic binding, per-source TTL |
| **T2** | ESP-NOW satellites, pairing flow, N up to 4, configurator source editor |
| **T3** | Dedicated hub board (option B) if user testing calls for it |
| **TX** | XAC build profile (section 2.2). Independent of T0-T3, gated on bench-verifying the constraints against a real XAC |

T1 removes the Titan Two from a PC setup. Removing it from a console setup is the AS-Bridge
track in [BRIDGE.md](BRIDGE.md), which shares only the T0 `src_id` dependency and otherwise runs
in parallel.

---

## 11. Open questions

1. How many sources is realistic? 4 is arbitrary. ESP-NOW peer limits and 1 kHz merge budget
   both need measuring before the number goes in a doc as a promise.
2. Should the hub be able to run on battery while satellites are wired, or is the hub always the
   mains/USB-powered node?
3. Does a satellite need its own web UI for calibration, or does the hub proxy configuration to
   it over AS-Link? Proxying is nicer for the user and more work for us.
4. If the hub dies, should satellites fall back to acting as standalone `STICK` devices? For
   gaming this is a nice recovery. For the DRIVE build it is exactly the kind of silent role
   change that build profile exists to forbid.
5. Does a real XAC actually accept a single-function HID joystick built to section 2.2, and does
   it pass analog axes or only the 8 buttons? Everything in 2.1 and 2.2 rests on third-party
   teardowns. This is the highest-value Phase 0 bench test in this document.
6. If the XAC path holds up, is a two-stick rig even possible through it? The XAC's two USB
   ports each replace one thumbstick, so two satellites merged into one hub output would arrive
   on a single port as a single stick. A two-port build may need two hubs, which defeats the
   merge entirely.
