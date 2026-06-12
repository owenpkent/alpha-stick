# Contributing to Alpha Stick

Thank you for your interest in contributing! Alpha Stick is an open-source project to make adaptive gaming more accessible.

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

```cpp
// Good
int16_t calculateDeadzone(int16_t rawValue, float deadzonePercent) {
    float normalized = rawValue / 32767.0f;
    if (fabs(normalized) < deadzonePercent) {
        return 0;
    }
    return rawValue;
}

// Avoid
int calc(int v, float d) { return abs(v/32767.0)<d?0:v; }
```

### 3D Models

- Include source files (FreeCAD, Fusion 360)
- Export STL with clear naming: `part-name-vX.stl`
- Use metric units (mm)
- Design for FDM printing (no supports preferred)

### Documentation

- Use Markdown
- Include diagrams where helpful
- Keep instructions step-by-step
- Test instructions before submitting

---

## Pull Request Guidelines

### Before Submitting

- [ ] Code compiles without errors
- [ ] Tested on real hardware (if applicable)
- [ ] Updated relevant documentation
- [ ] Added yourself to contributors (optional)

### PR Description Template

```markdown
## Description
Brief description of changes.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Hardware design

## Testing
How did you test this?

## Screenshots (if applicable)
```

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
]]>
