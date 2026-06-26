# Contributing to Alpha Stick

Thank you for your interest in contributing! Alpha Stick is an open-source project to make adaptive gaming more accessible.

New here? Read [docs/EXECUTIVE_SUMMARY.md](docs/EXECUTIVE_SUMMARY.md) for the
what-and-why, then [docs/WORKFLOW.md](docs/WORKFLOW.md) for how work happens
in this repo. This project follows our [Code of Conduct](CODE_OF_CONDUCT.md).

---

## Where to start (good first contributions)

The project is in Phase 0 (bench validation); [TODO.md](TODO.md) is the live
list. Contributions that help most right now:

1. **First hardware bring-up.** The firmware compiles in CI but has never run
   on a device: flash a bare ESP32-S3 devkit and confirm the simulation-mode
   gamepad demo (`firmware/README.md` checklist), then report what you saw.
2. **Print and report.** Print the primary **Tetra II flexure**
   ([models/tetra2-flexure/](models/tetra2-flexure/)) and/or the alternative pod
   v0 parts ([models/](models/)), try the feel, and file a Build/Bench Report
   issue with your printer, material, settings, and measured force.
3. **Verify the TMAG5273 register map** in
   `firmware/components/as_sensing/tmag5273.cpp` against the TI datasheet.
4. **Design toppers.** Custom toppers in `models/community/` (hollow, under
   1 g, include the printed mass).
5. **Platform testing.** Generic HID stick behavior on the Xbox Adaptive
   Controller's USB ports is designed-for but unverified.

---

## Ways to Contribute

### 🔧 Hardware

- **3D Model improvements** — Better ergonomics, mounting options
- **PCB design** — KiCad schematics and layouts
- **Component research** — Test and recommend parts
- **Alternative designs** — Mouth-operated, foot pedal, etc.

### 💻 Firmware

- **Feature development** — New input modes, protocols
- **Bug fixes** — Stability improvements
- **Testing** — Platform compatibility, edge cases
- **Optimization** — Power consumption, latency

### 📖 Documentation

- **Tutorials** — Written or video guides
- **Translations** — Make docs accessible in more languages
- **Diagrams** — Wiring, assembly illustrations
- **User guides** — End-user documentation

### 🧪 Testing

- **Usability testing** — Especially valuable from users with disabilities
- **Compatibility testing** — Different games, platforms
- **Hardware testing** — Component alternatives
- **Stress testing** — Long-term reliability

---

## Getting Started

### 1. Fork the Repository

Click "Fork" on GitHub to create your own copy.

### 2. Clone Your Fork

```powershell
git clone https://github.com/YOUR-USERNAME/alpha-stick.git
cd alpha-stick
```

### 3. Create a Branch

```powershell
git checkout -b feature/my-new-feature
```

Use descriptive branch names:
- `feature/add-bluetooth-pairing`
- `fix/joystick-drift`
- `docs/assembly-video`

### 4. Make Changes

Follow the coding conventions and test your changes.

### 5. Commit

Use conventional commit messages:

```powershell
git add -A
git commit -m "feat: add Bluetooth pairing button combo"
```

**Commit types:**
- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation
- `refactor:` Code restructuring
- `chore:` Maintenance

### 6. Push and Create PR

```powershell
git push origin feature/my-new-feature
```

Then open a Pull Request on GitHub.

---

## Code Conventions

### Firmware (C++)

- Use ESP-IDF 5.x conventions (FreeRTOS tasks, esp_err_t returns, components)
- Meaningful variable names
- Comment complex logic
- Keep functions focused and small
- Use `#pragma once` in headers

Match the existing scaffold style: `as::` namespace, snake_case functions,
one `components/as_*` directory per subsystem (see
`firmware/components/as_pipeline/` for the house style). New behavior that
differs between the gaming and drive builds is expressed in Kconfig, never as
a runtime flag.

```cpp
// Good (matches the scaffold)
float apply_radial_deadzone(float magnitude, float inner, float outer);

// Avoid
int calc(int v, float d) { return abs(v/32767.0)<d?0:v; }
```

### 3D Models

- The primary pivot, the **Tetra II flexure**, is a third-party STEP/STL in
  [`models/tetra2-flexure/`](models/tetra2-flexure/) (CC-BY): edit the STEP and
  re-export the STL, and keep its upstream attribution intact
- The alternative pod mechanism's source of truth is
  `models/source/pod-v0.scad` (OpenSCAD, parametric): edit named parameters,
  never hand-edit STLs, and commit the `.scad` and regenerated STLs together
- New standalone parts (toppers, bodies): any CAD is welcome in
  `models/community/`, but include the source file and an STL
- Toppers must be hollow and under 1 g printed (the moving-mass budget in
  docs/DESIGN_V2.md section 3 is a hard limit); state the printed mass in
  your PR
- Use metric units (mm); design for FDM without supports

### Documentation

- Use Markdown
- Include diagrams where helpful
- Keep instructions step-by-step
- Test instructions before submitting

---

## Pull Request Guidelines

### Before Submitting

- [ ] Code compiles without errors (CI builds `firmware/` and renders
      `models/` automatically on PRs that touch them)
- [ ] Tested on real hardware (if applicable); measurements go in
      [docs/BENCH_LOG.md](docs/BENCH_LOG.md)
- [ ] Walked the change-propagation table in
      [docs/WORKFLOW.md](docs/WORKFLOW.md) section 5 so docs, CAD, and
      firmware stay in sync
- [ ] Added yourself to contributors (optional)

The PR description form is provided automatically
(`.github/PULL_REQUEST_TEMPLATE.md`).

### Review Process

1. Submit PR
2. Maintainers review within a few days
3. Address feedback if any
4. PR merged when approved

---

## Accessibility Considerations

When contributing, keep accessibility in mind:

- **Physical:** Designs should accommodate limited strength/dexterity
- **Visual:** Documentation should be screen-reader friendly
- **Cognitive:** Instructions should be clear and not overwhelming

If you have a disability and want to share feedback, we especially value your input.

---

## Community Guidelines

- **Be respectful** — Treat everyone with kindness
- **Be patient** — This is a volunteer project
- **Be constructive** — Focus on solutions, not problems
- **Be inclusive** — Gaming is for everyone

The full standard is the [Code of Conduct](CODE_OF_CONDUCT.md), which applies
in all project spaces.

---

## Questions?

- **GitHub Issues** — For bugs, features, questions
- **Discussions** — For general conversation
- **Email** — For private matters

---

## License

By contributing, you agree that your contributions will be licensed under:
- **MIT License** — For code and firmware
- **CERN-OHL-P v2** — For hardware designs

---

Thank you for helping make gaming more accessible! 🎮
