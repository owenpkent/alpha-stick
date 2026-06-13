<div align="center">
  <h1>🎮 Alpha Stick</h1>
  <p><strong>Open-Source Adaptive Gaming Joystick</strong></p>
  <p>
    A 3D-printable, ESP32-powered joystick designed for gamers with limited mobility.<br/>
    Inspired by commercial solutions like QuadStick and Feather — but fully open and hackable.
  </p>
</div>

---

**New here?** Read the [Executive Summary](docs/EXECUTIVE_SUMMARY.md) (one page) or the [Whitepaper](docs/WHITEPAPER.md) (the full technical story). Want to help? See [CONTRIBUTING.md](CONTRIBUTING.md).

## Status

**Phase 0: Bench Validation** — V2 design baseline complete; proving the physics (force, noise, latency) before any product claims.

| Area | Status |
|------|--------|
| V2 First-Principles Design | 🔄 Baseline written: [docs/DESIGN_V2.md](docs/DESIGN_V2.md) |
| Hardware Design | ⏳ Planning |
| 3D Print Files | ⏳ Planning |
| Firmware | ⏳ Planning |
| Documentation | 🔄 In Progress |
| Project Nimbus Integration | ⏳ Planned |

---

## Why Alpha Stick?

Commercial adaptive controllers are **expensive** ($200–$500+) and often **proprietary**. Gamers with disabilities deserve affordable, customizable options they can build, modify, and repair themselves.

**Alpha Stick aims to provide:**

- **Affordable** — 3D-printed enclosure + off-the-shelf electronics (~$30–50 BOM)
- **Open Source** — Hardware, firmware, and documentation all freely available
- **Customizable** — Modular design adaptable to different needs and abilities
- **Updateable** — OTA firmware updates via ESP32
- **Compatible** — Works with Xbox Adaptive Controller, PlayStation Access Controller, PC, and [Project Nimbus](https://github.com/owenpkent/Project-Nimbus)

---

## Inspiration

Alpha Stick draws from the best features of existing adaptive controllers:

| Product | Key Features We're Inspired By |
|---------|-------------------------------|
| **[QuadStick](https://www.quadstick.com/)** | Mouth-operated joystick, sip/puff input, multi-platform support |
| **[Feather](https://www.celticmagic.org/feather)** | Ultra-light force (<5g), magnetic sensing, tremor filter, adjustable sensitivity |
| **[Xbox Adaptive Controller](https://www.xbox.com/en-US/accessories/controllers/xbox-adaptive-controller)** | 3.5mm jack inputs, USB hub, profile switching, 3D-printable accessories |
| **[PlayStation Access](https://www.playstation.com/en-us/accessories/access-controller/)** | Swappable button caps, adjustable stick length, expandable inputs |
| **[Titan Two](https://www.consoletuner.com/products/titan-two/)** | Cross-platform compatibility, scripting, keyboard/mouse support |

---

## Planned Features

### Hardware
- **ESP32-S3** microcontroller with Bluetooth LE and USB HID
- **Hall-effect or analog joystick** — configurable sensitivity
- **Ultra-low force option** — magnetic sensing for minimal movement requirements
- **3.5mm input jacks** — connect external switches and buttons
- **Modular enclosure** — 3D-printed, designed for different mounting options
- **Sip/puff interface** (optional) — pressure sensor input

### Firmware
- **USB HID gamepad** — plug-and-play on PC/consoles
- **Bluetooth LE gamepad** — wireless connection
- **XInput/DirectInput modes** — broad game compatibility
- **OTA updates** — update firmware over WiFi
- **Web-based configuration** — adjust sensitivity, deadzone, button mapping via browser
- **Profile system** — save and switch between configurations
- **Tremor filter** — configurable smoothing for users with tremors

### Software Integration
- **Project Nimbus integration** — use Alpha Stick with the virtual controller interface
- **Xbox Adaptive Controller companion** — connect via 3.5mm or USB
- **Configuration app** — desktop/mobile app for settings (future)

---

## Target Platforms

- **PC** — Windows (USB/Bluetooth), Linux, macOS
- **Xbox** — Via Xbox Adaptive Controller (3.5mm jack or USB)
- **PlayStation** — Via PS Access Controller or adapter
- **Nintendo Switch** — Via USB or Bluetooth (with appropriate mode)
- **Mobile** — Android/iOS via Bluetooth

---

## Project Structure

```
alpha-stick/
├── README.md                 # This file
├── LICENSE                   # MIT License
├── TODO.md                   # Task tracking
├── LLM_ONBOARDING.md         # AI assistant reference
├── docs/
│   ├── EXECUTIVE_SUMMARY.md  # One page: what and why
│   ├── WHITEPAPER.md         # The full technical narrative
│   ├── DESIGN_V2.md          # V2 design baseline (working doc)
│   ├── WORKFLOW.md           # Development loops & conventions
│   ├── HARDWARE.md           # Build reference & BOM
│   ├── FIRMWARE.md           # Firmware architecture (ESP-IDF)
│   ├── PHASE0_PARTS.md       # Bench parts list
│   ├── BENCH_LOG.md          # Measured results
│   ├── PRINTING.md           # 3D printing guide
│   ├── ASSEMBLY.md           # Assembly instructions
│   └── CONFIGURATION.md      # Configuration guide
├── hardware/
│   ├── schematics/           # KiCad or EasyEDA files
│   ├── pcb/                  # PCB design files
│   └── bom/                  # Bill of materials
├── firmware/
│   ├── main/                 # Entry point, mode supervisor
│   ├── components/           # Sensing, pipeline, HID, AS-Link, config
│   ├── web/                  # Web config UI source
│   └── sdkconfig.defaults    # ESP-IDF configuration
├── models/
│   ├── stl/                  # Ready-to-print STL files
│   ├── step/                 # STEP files for modification
│   └── source/               # FreeCAD/Fusion360 source files
├── software/
│   ├── web-config/           # Web-based configuration UI
│   └── tools/                # Utility scripts
└── examples/
    └── profiles/             # Example configuration profiles
```

---

## Hardware Overview

### Core Components

| Component | Purpose | Est. Cost |
|-----------|---------|-----------|
| ESP32-S3 module | Main controller | $4–10 |
| Sensor pod (dual Hall + magnets + pivot) | Position input, <5 g force | $6–8 |
| 3.5mm jacks (x4) | External switch inputs | $2 |
| Buttons (x2–4) | Direct button inputs | $1–2 |
| 3D-printed enclosure | Housing | $2–5 (filament) |
| USB-C connector | Power/data | $1 |
| Misc (wires, screws) | Assembly | $2–3 |
| **Total** | | **~$20–30** |

### Optional Add-ons

| Component | Purpose | Est. Cost |
|-----------|---------|-----------|
| Hall-effect joystick | Ultra-smooth input | $10–20 |
| Pressure sensor | Sip/puff interface | $5–10 |
| LiPo battery + charger | Wireless operation | $10–15 |
| OLED display | Status/config display | $5 |

---

## Getting Started

> **Note:** Hardware and firmware are still in development. Check back soon!

### Prerequisites

- **3D Printer** — Any FDM printer (Ender 3, Prusa, etc.)
- **Soldering iron** — Basic through-hole soldering
- **ESP-IDF 5.x** — For firmware development ([install](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html))
- **ESP32-S3 board** — DevKitC or similar

### Quick Start (Coming Soon)

```powershell
# Clone the repository
git clone https://github.com/owenpkent/alpha-stick.git

# Navigate to firmware directory
cd alpha-stick/firmware

# Build and flash (ESP-IDF PowerShell)
idf.py set-target esp32s3; idf.py build flash

# Open web configuration
# Connect to "AlphaStick" WiFi network, navigate to http://192.168.4.1
```

---

## Project Nimbus Integration

Alpha Stick is designed to work seamlessly with [**Project Nimbus**](https://github.com/owenpkent/Project-Nimbus) — a virtual controller interface for accessibility.

### Integration Options

1. **Direct USB** — Alpha Stick appears as a standard gamepad; Project Nimbus can combine it with mouse input
2. **Bluetooth** — Wireless connection to PC running Project Nimbus
3. **Hybrid mode** — Use Alpha Stick for analog input, Project Nimbus for buttons and additional controls
4. **Configuration sync** — Share sensitivity curves and profiles between projects

### Use Cases

- **Alpha Stick + Project Nimbus** — Physical joystick for one hand, on-screen buttons for the other
- **Dual Alpha Sticks** — Two joysticks for flight sim or dual-stick shooters
- **Alpha Stick + Xbox Adaptive Controller** — Use Alpha Stick as an external joystick input

---

## Accessibility Features

Designed with input from the disability gaming community:

- **Adjustable force sensitivity** — From standard to ultra-light (<10g)
- **Configurable deadzone** — Accommodate tremors and limited precision
- **Tremor filtering** — Smooth out unintended movements
- **Button debouncing** — Adjustable for different switch types
- **One-handed operation** — Compact form factor, all controls accessible
- **Voice/switch control** — External switch inputs for additional buttons
- **Custom mounting** — Designed for RAM mounts, Magic Arms, and wheelchair trays

---

## Contributing

We welcome contributions from everyone! Areas where we especially need help:

- **3D modeling** — Enclosure designs for different needs
- **PCB design** — If you have KiCad/EasyEDA experience
- **Firmware development** — ESP32, ESP-IDF, Bluetooth HID
- **Testing** — Especially from users with disabilities
- **Documentation** — Tutorials, translations, videos

### How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Make your changes with clear commit messages
4. Submit a pull request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

---

## Roadmap

### Phase 1: Foundation (Current)
- [ ] Define hardware requirements
- [ ] Select core components
- [ ] Create initial 3D model
- [ ] Basic firmware scaffold

### Phase 2: Prototype
- [ ] Working breadboard prototype
- [ ] USB HID functionality
- [ ] Basic web configuration
- [ ] First printable enclosure

### Phase 3: Refinement
- [ ] Bluetooth support
- [ ] OTA updates
- [ ] Sip/puff option
- [ ] Community testing

### Phase 4: Release
- [ ] Documentation complete
- [ ] Multiple enclosure options
- [ ] Profile library
- [ ] v1.0 release

---

## Community

- **GitHub Issues** — Bug reports, feature requests, questions
- **Discussions** — Share your builds, ask for help
- **Discord** — (Coming soon)

---

## Related Projects

- **[Project Nimbus](https://github.com/owenpkent/Project-Nimbus)** — Virtual controller interface for accessibility
- **ATOS** — Modular ESP32-S3 wheelchair platform; Alpha Stick V2 is designed to act as its input node (see [docs/DESIGN_V2.md](docs/DESIGN_V2.md), section 9)
- **[Joypad OS](https://github.com/joypad-ai/joypad-os)** — Universal controller firmware for adapters and protocol translation (reference architecture)
- **[Xbox Adaptive Controller](https://www.xbox.com/en-US/accessories/controllers/xbox-adaptive-controller)** — Microsoft's adaptive gaming hub
- **[QuadStick](https://www.quadstick.com/)** — Commercial mouth-operated controller
- **[Feather Joystick](https://www.celticmagic.org/feather)** — Ultra-light force joystick
- **[OpenSwitchBoard](https://github.com/makersmakingchange/OpenSwitchBoard)** — Open-source switch interface
- **[Makers Making Change](https://makersmakingchange.com/)** — Community building assistive technology

---

## License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) for details.

Hardware designs are released under **CERN-OHL-P v2** (permissive).

---

## Acknowledgments

- The disability gaming community for feedback and inspiration
- [AbleGamers](https://ablegamers.org/) and [SpecialEffect](https://www.specialeffect.org.uk/) for advocacy
- Owen — project creator, wheelchair user with muscular dystrophy
- Everyone building accessible gaming solutions

---

<div align="center">
  <strong>Gaming is for everyone. Let's build it together.</strong>
</div>
