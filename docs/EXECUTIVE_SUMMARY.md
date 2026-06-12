# Alpha Stick: Executive Summary

**An open-source gaming joystick that moves with less than 5 grams of force,
built from $25 of parts, that is also a mouse and a wheelchair input device.**

June 2026 | Owen Kent | MIT licensed firmware, CERN-OHL-P hardware

---

## The problem

For people with muscular dystrophy, SMA, ALS, and similar conditions, a
standard game controller thumbstick is a wall: it takes 150 to 450 grams of
force to deflect. The few devices built for ultra-low force are excellent but
expensive (commercial adaptive controllers run $200 to $500+), proprietary,
and single-purpose. The input device is the hardest and most personal part of
every assistive setup, and today it cannot be repaired, adapted, or afforded
by most of the people who need it.

## The solution

Alpha Stick attacks the force problem at the physics level instead of
softening a conventional joystick:

- **Nothing touches anything.** Position is sensed contactlessly (a magnet
  read by Hall-effect sensors), and the centering force is magnetic
  attraction rather than a spring. The only contact is a pivot with under
  0.2 grams of friction.
- **The force is a dial.** Turning a threaded magnet carrier sets the
  full-deflection force anywhere from 1 to 8 grams, with no tools, so the
  stick can match a user's strength day to day.
- **One device, many roles.** The same hardware is a USB/Bluetooth gamepad, a
  precision mouse (including zero-force dwell clicking), a keyboard, and an
  input node for the ATOS open wheelchair platform.
- **Anyone can build it.** A 3D printer and a soldering iron; the core bill
  of materials is about $22.

## What is novel

1. **Direction-based magnetic sensing** makes the measurement immune to
   temperature and mechanical creep, which is what makes a sub-5-gram printed
   device stable enough to trust.
2. **Adjustable passive magnetic centering**: one magnet provides the pivot
   preload and the return force simultaneously, with zero springs and zero
   wearing parts.
3. **A drive-grade upgrade path**: dual redundant sensors (about $1 of
   silicon) plus a TTL/sequence-numbered protocol let the same stick serve as
   a wheelchair drive input under a safety host's authority.
4. **A research stage no commercial device offers**: planar coils under the
   magnet for firmware-programmable force, virtual detents, and active tremor
   damping at the physical layer.

## Status (June 2026)

| Piece | State |
|-------|-------|
| Design baseline (physics, budgets, architecture) | Complete |
| Firmware (ESP-IDF, USB composite, processing pipeline) | Scaffold complete, builds pending hardware bring-up |
| Bench rig CAD (parametric, printable) | Complete |
| Bench validation (force, noise, latency) | Starting: parts list ready, nothing measured yet |

The project is deliberately at the "prove the numbers" stage: every estimate
has a written target and a bench plan before any product claims are made.

## Roadmap

| Stage | Contents |
|-------|----------|
| A "Glide" | The core stick: USB + Bluetooth gamepad/mouse, web configuration |
| B "Bridge" | Switch jacks, sip/puff, chin/mouth/thumb form factors, ATOS link |
| C "Field" | Active coils: programmable feel, haptics, physical tremor damping |

## The ATOS connection

ATOS (the open wheelchair platform in the RAMMP program) defines how input
devices propose motion to a safety-critical host but has no input device built
yet. Alpha Stick is designed to be that device: it proposes, the wheelchair's
safety host disposes, and the gaming and drive firmware are separate builds
held to different standards.

## How to help

3D modeling of form factors, PCB review, firmware (ESP-IDF), and most of all
**testing by people with disabilities**, whose feedback outranks every
estimate in this repository. The project lives at
github.com/owenpkent/alpha-stick under MIT (firmware) and CERN-OHL-P
(hardware) licenses.
