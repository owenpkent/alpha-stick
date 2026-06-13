"""Alpha Stick profile model (host side).

A profile is a plain dict matching the JSON schema in docs/CONFIGURATION.md.
The firmware implements the stick/mouse subset today (mount, deadzone, curve,
filter, mouse); the `buttons` and `macros` sections are authored and stored
here now and will be consumed once the input subsystem and macro engine land
in firmware. Sending a full profile to the device is safe: it merges the keys
it knows and ignores the rest.
"""

from __future__ import annotations

import copy
import json
from typing import Any

# --- enumerations the GUI offers -----------------------------------------

MODES = ["gamepad", "mouse", "keyboard", "dual", "atos"]
CURVES = ["linear", "expo", "log", "scurve"]

# Physical input sources (docs/CONFIGURATION.md "Buttons, Jacks, Sip/Puff").
INPUT_SOURCES = [
    "button1", "button2",
    "jack1", "jack2", "jack3", "jack4",
    "zpress",
    "sip_soft", "sip_hard", "puff_soft", "puff_hard",
]

# What a source can do when triggered.
ACTION_TYPES = ["none", "gamepad_button", "key", "mouse_button", "macro"]
BEHAVIORS = ["momentary", "toggle"]

# Step kinds for a macro.
MACRO_STEP_TYPES = ["key", "gamepad_button", "mouse_button", "delay"]
MACRO_STEP_ACTIONS = ["tap", "press", "release"]  # ignored for "delay"


def default_profile() -> dict[str, Any]:
    """A fresh default profile (mirrors the firmware Profile defaults)."""
    return {
        "name": "default",
        "mode": "gamepad",
        "mount": {
            "rotation_deg": 0.0,
            "gravity_tare": [0.0, 0.0],
            "invert_x": False,
            "invert_y": False,
        },
        "deadzone": {"type": "radial", "inner": 0.04, "outer": 0.98},
        "curve": {"type": "expo", "gain": 1.0, "expo": 0.4},
        "filter": {"one_euro": {"min_cutoff": 1.0, "beta": 0.02}, "notch_hz": 0},
        "mouse": {"max_px_s": 900.0},
        # Authored here, staged for firmware (see module docstring).
        "buttons": {src: empty_binding() for src in INPUT_SOURCES},
        "macros": [],
    }


def empty_binding() -> dict[str, Any]:
    return {"action": "none", "code": "", "behavior": "momentary"}


def empty_macro(name: str = "macro") -> dict[str, Any]:
    return {"name": name, "steps": []}


def empty_step() -> dict[str, Any]:
    return {"type": "key", "action": "tap", "code": "", "delay_ms": 0}


def merge_defaults(profile: dict[str, Any]) -> dict[str, Any]:
    """Return a copy of `profile` with any missing keys filled from defaults.

    Keeps older or partial profile files loadable as the schema grows.
    """
    base = default_profile()
    out = _deep_merge(base, profile)
    # Ensure every known source has a binding entry.
    buttons = out.setdefault("buttons", {})
    for src in INPUT_SOURCES:
        buttons.setdefault(src, empty_binding())
    return out


def _deep_merge(base: dict[str, Any], over: dict[str, Any]) -> dict[str, Any]:
    result = copy.deepcopy(base)
    for key, value in (over or {}).items():
        if isinstance(value, dict) and isinstance(result.get(key), dict):
            result[key] = _deep_merge(result[key], value)
        else:
            result[key] = copy.deepcopy(value)
    return result


def load(path: str) -> dict[str, Any]:
    with open(path, "r", encoding="utf-8") as fh:
        data = json.load(fh)
    if not isinstance(data, dict):
        raise ValueError("profile file is not a JSON object")
    return merge_defaults(data)


def save(path: str, profile: dict[str, Any]) -> None:
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(profile, fh, indent=2)
        fh.write("\n")


def validate(profile: dict[str, Any]) -> list[str]:
    """Return a list of human-readable problems; empty means valid."""
    problems: list[str] = []
    if profile.get("mode") not in MODES:
        problems.append(f"mode must be one of {MODES}")
    dz = profile.get("deadzone", {})
    inner, outer = dz.get("inner", 0), dz.get("outer", 1)
    if not (0.0 <= inner < outer <= 1.0):
        problems.append("deadzone needs 0 <= inner < outer <= 1")
    if profile.get("curve", {}).get("type") not in CURVES:
        problems.append(f"curve type must be one of {CURVES}")
    for i, macro in enumerate(profile.get("macros", [])):
        for j, step in enumerate(macro.get("steps", [])):
            if step.get("type") not in MACRO_STEP_TYPES:
                problems.append(f"macro {i} step {j}: bad type {step.get('type')!r}")
    return problems
