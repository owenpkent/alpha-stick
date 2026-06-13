#!/usr/bin/env python3
"""Alpha Stick configurator (end-user GUI).

A desktop app to read, edit, and apply Alpha Stick settings over USB: axes,
deadzone, response curve, tremor filter, mouse speed, button mappings, and
macros. Stick/mouse settings apply live; button mappings and macros are
authored and saved to profile files now and take effect once the firmware
input subsystem lands.

Run:  python tools/configurator.py
Needs Python 3.9+ with tkinter (bundled on Windows/macOS python.org builds and
most Linux distros) and pyserial (pip install -r tools/requirements.txt).
"""

from __future__ import annotations

import json
import os
import sys
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from alphastick import profile as pm  # noqa: E402
from alphastick.protocol import (  # noqa: E402
    AlphaStick,
    DeviceError,
    find_port,
    list_candidate_ports,
)


def _get_path(d: dict, path: str):
    cur = d
    for key in path.split("."):
        cur = cur[key]
    return cur


def _set_path(d: dict, path: str, value) -> None:
    keys = path.split(".")
    cur = d
    for key in keys[:-1]:
        cur = cur.setdefault(key, {})
    cur[keys[-1]] = value


MACRO_HELP = (
    "Macros are a JSON list. Each macro has a name and ordered steps.\n"
    'Step types: "key", "gamepad_button", "mouse_button" (with '
    '"action": tap/press/release and a "code"), or "delay" (with "delay_ms").\n'
    "Authored here now; the firmware runs them once the input subsystem lands."
)


class ConfiguratorApp:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.dev: AlphaStick | None = None
        self.profile = pm.default_profile()
        self.current_path: str | None = None

        self.vars: dict[str, tk.Variable] = {}
        self.button_vars: dict[str, dict[str, tk.Variable]] = {}
        self.device_buttons: list[ttk.Button] = []

        self._build_style()
        self._build_ui()
        self._refresh_ports()
        self._load_profile_into_widgets()
        self._set_connected(False)
        self._status("Not connected. Pick a port and Connect, or work offline and Save to a file.")

    # --- UI construction --------------------------------------------------

    def _build_style(self) -> None:
        style = ttk.Style()
        default_font = ("Segoe UI", 11) if sys.platform == "win32" else ("TkDefaultFont", 11)
        self.root.option_add("*Font", default_font)
        style.configure("TButton", padding=4)
        style.configure("Status.TLabel", foreground="#333")

    def _build_ui(self) -> None:
        self.root.title("Alpha Stick Configurator")
        self.root.minsize(640, 560)

        self._build_connection_bar()

        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill="both", expand=True, padx=10, pady=(0, 6))
        self._build_stick_tab()
        self._build_mouse_tab()
        self._build_buttons_tab()
        self._build_macros_tab()

        self._build_action_bar()

        self.status_var = tk.StringVar()
        ttk.Label(self.root, textvariable=self.status_var, style="Status.TLabel",
                  anchor="w").pack(fill="x", padx=10, pady=(0, 8))

    def _build_connection_bar(self) -> None:
        bar = ttk.Frame(self.root)
        bar.pack(fill="x", padx=10, pady=10)
        ttk.Label(bar, text="Port:").pack(side="left")
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(bar, textvariable=self.port_var, width=34,
                                       state="readonly")
        self.port_combo.pack(side="left", padx=6)
        ttk.Button(bar, text="Refresh", command=self._refresh_ports).pack(side="left")
        self.connect_btn = ttk.Button(bar, text="Connect", command=self._toggle_connect)
        self.connect_btn.pack(side="left", padx=6)
        self.device_label = ttk.Label(bar, text="")
        self.device_label.pack(side="left", padx=10)

    def _labeled_slider(self, parent, label, path, frm, to):
        row = ttk.Frame(parent)
        row.pack(fill="x", pady=4)
        ttk.Label(row, text=label, width=20, anchor="w").pack(side="left")
        var = tk.DoubleVar()
        self.vars[path] = var
        scale = ttk.Scale(row, from_=frm, to=to, orient="horizontal", variable=var,
                          length=240)
        scale.pack(side="left", padx=6)
        entry = ttk.Entry(row, textvariable=var, width=8)
        entry.pack(side="left")
        return var

    def _labeled_combo(self, parent, label, path, values):
        row = ttk.Frame(parent)
        row.pack(fill="x", pady=4)
        ttk.Label(row, text=label, width=20, anchor="w").pack(side="left")
        var = tk.StringVar()
        self.vars[path] = var
        ttk.Combobox(row, textvariable=var, values=values, state="readonly",
                     width=16).pack(side="left", padx=6)
        return var

    def _labeled_check(self, parent, label, path):
        var = tk.BooleanVar()
        self.vars[path] = var
        ttk.Checkbutton(parent, text=label, variable=var).pack(anchor="w", pady=2)
        return var

    def _build_stick_tab(self) -> None:
        tab = ttk.Frame(self.notebook, padding=12)
        self.notebook.add(tab, text="Stick")

        row = ttk.Frame(tab)
        row.pack(fill="x", pady=4)
        ttk.Label(row, text="Profile name", width=20, anchor="w").pack(side="left")
        name_var = tk.StringVar()
        self.vars["name"] = name_var
        ttk.Entry(row, textvariable=name_var, width=22).pack(side="left", padx=6)

        self._labeled_combo(tab, "Mode", "mode", pm.MODES)
        ttk.Separator(tab, orient="horizontal").pack(fill="x", pady=8)

        self._labeled_check(tab, "Invert X", "mount.invert_x")
        self._labeled_check(tab, "Invert Y", "mount.invert_y")
        self._labeled_slider(tab, "Rotation (deg)", "mount.rotation_deg", -180, 180)
        ttk.Separator(tab, orient="horizontal").pack(fill="x", pady=8)

        self._labeled_slider(tab, "Deadzone inner", "deadzone.inner", 0.0, 0.5)
        self._labeled_slider(tab, "Deadzone outer", "deadzone.outer", 0.5, 1.0)
        ttk.Separator(tab, orient="horizontal").pack(fill="x", pady=8)

        self._labeled_combo(tab, "Curve", "curve.type", pm.CURVES)
        self._labeled_slider(tab, "Curve gain", "curve.gain", 0.1, 3.0)
        self._labeled_slider(tab, "Curve expo", "curve.expo", 0.0, 1.0)
        ttk.Separator(tab, orient="horizontal").pack(fill="x", pady=8)

        self._labeled_slider(tab, "Filter min cutoff", "filter.one_euro.min_cutoff", 0.0, 5.0)
        self._labeled_slider(tab, "Filter beta", "filter.one_euro.beta", 0.0, 0.2)

    def _build_mouse_tab(self) -> None:
        tab = ttk.Frame(self.notebook, padding=12)
        self.notebook.add(tab, text="Mouse")
        ttk.Label(tab, text="Pointer speed at full deflection (px/s).").pack(anchor="w")
        self._labeled_slider(tab, "Max px/s", "mouse.max_px_s", 100, 2000)

    def _build_buttons_tab(self) -> None:
        tab = ttk.Frame(self.notebook, padding=12)
        self.notebook.add(tab, text="Buttons")
        ttk.Label(
            tab,
            text="Map each input to an action. Saved to the profile now; the "
                 "firmware applies these once the input subsystem lands.",
            wraplength=580, foreground="#555",
        ).pack(anchor="w", pady=(0, 8))

        header = ttk.Frame(tab)
        header.pack(fill="x")
        ttk.Label(header, text="Source", width=14, anchor="w").pack(side="left")
        ttk.Label(header, text="Action", width=16, anchor="w").pack(side="left")
        ttk.Label(header, text="Code", width=10, anchor="w").pack(side="left")
        ttk.Label(header, text="Behavior", width=12, anchor="w").pack(side="left")

        for src in pm.INPUT_SOURCES:
            row = ttk.Frame(tab)
            row.pack(fill="x", pady=1)
            ttk.Label(row, text=src, width=14, anchor="w").pack(side="left")
            action_var = tk.StringVar()
            code_var = tk.StringVar()
            behavior_var = tk.StringVar()
            ttk.Combobox(row, textvariable=action_var, values=pm.ACTION_TYPES,
                         state="readonly", width=14).pack(side="left")
            ttk.Entry(row, textvariable=code_var, width=10).pack(side="left", padx=4)
            ttk.Combobox(row, textvariable=behavior_var, values=pm.BEHAVIORS,
                         state="readonly", width=11).pack(side="left")
            self.button_vars[src] = {
                "action": action_var, "code": code_var, "behavior": behavior_var,
            }

    def _build_macros_tab(self) -> None:
        tab = ttk.Frame(self.notebook, padding=12)
        self.notebook.add(tab, text="Macros")
        ttk.Label(tab, text=MACRO_HELP, wraplength=580, foreground="#555",
                  justify="left").pack(anchor="w", pady=(0, 8))

        btns = ttk.Frame(tab)
        btns.pack(fill="x", pady=(0, 6))
        ttk.Button(btns, text="Insert macro template",
                   command=self._insert_macro_template).pack(side="left")
        ttk.Button(btns, text="Validate", command=self._validate_macros).pack(side="left", padx=6)

        self.macro_text = tk.Text(tab, height=14, wrap="none", undo=True)
        self.macro_text.pack(fill="both", expand=True)

    def _build_action_bar(self) -> None:
        bar = ttk.Frame(self.root)
        bar.pack(fill="x", padx=10, pady=6)

        def dev_button(text, cmd):
            b = ttk.Button(bar, text=text, command=cmd)
            b.pack(side="left", padx=3)
            self.device_buttons.append(b)
            return b

        dev_button("Read from device", self._read_from_device)
        dev_button("Apply (live)", self._apply_to_device)
        dev_button("Save on device", self._save_on_device)
        dev_button("Reboot", self._reboot_device)

        ttk.Separator(bar, orient="vertical").pack(side="left", fill="y", padx=8)
        ttk.Button(bar, text="Open file...", command=self._open_file).pack(side="left", padx=3)
        ttk.Button(bar, text="Save file...", command=self._save_file).pack(side="left", padx=3)
        ttk.Button(bar, text="Reset defaults", command=self._reset_defaults).pack(side="left", padx=3)

    # --- profile <-> widgets ---------------------------------------------

    def _load_profile_into_widgets(self) -> None:
        self.profile = pm.merge_defaults(self.profile)
        for path, var in self.vars.items():
            try:
                var.set(_get_path(self.profile, path))
            except (KeyError, tk.TclError):
                pass
        for src, group in self.button_vars.items():
            binding = self.profile.get("buttons", {}).get(src, pm.empty_binding())
            group["action"].set(binding.get("action", "none"))
            group["code"].set(str(binding.get("code", "")))
            group["behavior"].set(binding.get("behavior", "momentary"))
        self.macro_text.delete("1.0", "end")
        self.macro_text.insert("1.0", json.dumps(self.profile.get("macros", []), indent=2))

    def _collect_widgets_into_profile(self) -> bool:
        for path, var in self.vars.items():
            try:
                value = var.get()
            except tk.TclError:
                self._status(f"'{path}' has an invalid number; fix it and retry.")
                return False
            _set_path(self.profile, path, value)
        buttons = self.profile.setdefault("buttons", {})
        for src, group in self.button_vars.items():
            buttons[src] = {
                "action": group["action"].get() or "none",
                "code": group["code"].get(),
                "behavior": group["behavior"].get() or "momentary",
            }
        try:
            macros = json.loads(self.macro_text.get("1.0", "end").strip() or "[]")
            if not isinstance(macros, list):
                raise ValueError("macros must be a JSON list")
            self.profile["macros"] = macros
        except (json.JSONDecodeError, ValueError) as exc:
            self._status(f"Macros JSON problem: {exc}")
            return False
        return True

    # --- macro helpers ----------------------------------------------------

    def _insert_macro_template(self) -> None:
        try:
            macros = json.loads(self.macro_text.get("1.0", "end").strip() or "[]")
            if not isinstance(macros, list):
                macros = []
        except json.JSONDecodeError:
            macros = []
        macros.append({
            "name": f"macro{len(macros) + 1}",
            "steps": [
                {"type": "key", "action": "tap", "code": "A"},
                {"type": "delay", "delay_ms": 100},
            ],
        })
        self.macro_text.delete("1.0", "end")
        self.macro_text.insert("1.0", json.dumps(macros, indent=2))

    def _validate_macros(self) -> None:
        if not self._collect_widgets_into_profile():
            return
        problems = pm.validate(self.profile)
        if problems:
            messagebox.showwarning("Validation", "\n".join(problems))
        else:
            self._status("Profile valid.")

    # --- device actions ---------------------------------------------------

    def _refresh_ports(self) -> None:
        ports = list_candidate_ports()
        labels = [desc for _dev, desc in ports]
        self._port_map = {desc: dev for dev, desc in ports}
        self.port_combo["values"] = labels
        if labels and not self.port_var.get():
            self.port_combo.current(0)

    def _selected_port(self) -> str | None:
        label = self.port_var.get()
        if not label:
            return None
        return self._port_map.get(label, label)

    def _toggle_connect(self) -> None:
        if self.dev:
            self.dev.close()
            self.dev = None
            self._set_connected(False)
            self.device_label.config(text="")
            self._status("Disconnected.")
            return
        port = self._selected_port() or find_port()
        try:
            self.dev = AlphaStick(port)
            info = self.dev.info()
        except DeviceError as exc:
            self.dev = None
            messagebox.showerror("Connect", str(exc))
            return
        self._set_connected(True)
        self.device_label.config(text=f"{info.get('device')}  fw {info.get('fw')}")
        self._status("Connected. Reading current profile from the device...")
        self._read_from_device()

    def _set_connected(self, connected: bool) -> None:
        self.connect_btn.config(text="Disconnect" if connected else "Connect")
        state = "normal" if connected else "disabled"
        for btn in self.device_buttons:
            btn.config(state=state)

    def _require_device(self) -> bool:
        if not self.dev:
            messagebox.showinfo("Not connected", "Connect to a device first.")
            return False
        return True

    def _read_from_device(self) -> None:
        if not self._require_device():
            return
        try:
            dev_profile = self.dev.get_profile()
        except DeviceError as exc:
            messagebox.showerror("Read", str(exc))
            return
        merged = pm.merge_defaults(dev_profile)
        # Keep host-only authoring that the device does not round-trip.
        merged["buttons"] = self.profile.get("buttons", merged["buttons"])
        merged["macros"] = self.profile.get("macros", merged["macros"])
        self.profile = merged
        self._load_profile_into_widgets()
        self._status("Loaded profile from device.")

    def _apply_to_device(self) -> None:
        if not self._require_device():
            return
        if not self._collect_widgets_into_profile():
            return
        problems = pm.validate(self.profile)
        if problems:
            messagebox.showwarning("Validation", "\n".join(problems))
            return
        try:
            self.dev.set_profile(self.profile)
        except DeviceError as exc:
            messagebox.showerror("Apply", str(exc))
            return
        self._status("Applied to device (stick/mouse settings are live now).")

    def _save_on_device(self) -> None:
        if not self._require_device():
            return
        if not self._collect_widgets_into_profile():
            return
        try:
            self.dev.set_profile(self.profile)
            self.dev.save()
        except DeviceError as exc:
            messagebox.showerror("Save", str(exc))
            return
        self._status("Saved on device (persists across reboots).")

    def _reboot_device(self) -> None:
        if not self._require_device():
            return
        if not messagebox.askyesno("Reboot", "Reboot the device now?"):
            return
        self.dev.reboot()
        self._status("Reboot requested.")

    # --- file actions -----------------------------------------------------

    def _open_file(self) -> None:
        path = filedialog.askopenfilename(
            title="Open profile", filetypes=[("Profile JSON", "*.json"), ("All", "*.*")])
        if not path:
            return
        try:
            self.profile = pm.load(path)
        except (OSError, ValueError, json.JSONDecodeError) as exc:
            messagebox.showerror("Open", str(exc))
            return
        self.current_path = path
        self._load_profile_into_widgets()
        self._status(f"Opened {os.path.basename(path)}.")

    def _save_file(self) -> None:
        if not self._collect_widgets_into_profile():
            return
        path = filedialog.asksaveasfilename(
            title="Save profile", defaultextension=".json",
            initialfile=(self.profile.get("name", "profile") + ".json"),
            filetypes=[("Profile JSON", "*.json")])
        if not path:
            return
        try:
            pm.save(path, self.profile)
        except OSError as exc:
            messagebox.showerror("Save", str(exc))
            return
        self.current_path = path
        self._status(f"Saved {os.path.basename(path)}.")

    def _reset_defaults(self) -> None:
        if not messagebox.askyesno("Reset", "Reset all fields to defaults? "
                                   "This does not touch the device until you Apply."):
            return
        self.profile = pm.default_profile()
        self._load_profile_into_widgets()
        self._status("Reset to defaults.")

    # --- misc -------------------------------------------------------------

    def _status(self, msg: str) -> None:
        self.status_var.set(msg)


def main() -> int:
    root = tk.Tk()
    ConfiguratorApp(root)
    root.mainloop()
    return 0


if __name__ == "__main__":
    sys.exit(main())
