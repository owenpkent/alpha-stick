# Security Policy

Alpha Stick is an open-source hardware + firmware project for adaptive gaming.
Security and safety matter here in particular because the same input core is
designed to *propose* motion to a wheelchair host (see
[DESIGN_V2.md](docs/DESIGN_V2.md) section 9).

## Supported versions

The project is in **Phase 0 (bench validation)** and has not cut a tagged
release. Until one exists, the `main` branch is the only supported version.

## Reporting a vulnerability

Please report security issues **privately** — do not open a public issue:

- **Preferred:** open a private advisory via **Security → Report a
  vulnerability** ([GitHub Security Advisories](https://github.com/owenpkent/alpha-stick/security/advisories/new)).
- **Or** email **owenpkent@gmail.com** with `Alpha Stick security` in the
  subject.

Include what you found, how to reproduce it, and the impact you expect. You
will get an acknowledgement within a few days. There is no bounty program; with
your permission you will be credited in the advisory.

## Scope

In scope:

- Firmware (`firmware/`): USB/BLE HID, the USB CDC config protocol, OTA, and the
  AS-Link drive protocol.
- Configuration tooling (`tools/`).

**Safety note:** the drive (wheelchair) path is **propose-only** — the ATOS host
owns the final motor write and the physical e-stop (DESIGN_V2 section 9). The
failure mode we care about most is *any* way the stick could assert motion
authority on its own; please report those even if they feel theoretical.

Out of scope:

- Issues that require physical access to an unlocked device you already own.
- Third-party dependencies — report those upstream, but tell us which dependency
  and version so we can pin or patch.
