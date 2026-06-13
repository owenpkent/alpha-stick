#!/usr/bin/env python3
"""Alpha Stick developer firmware tool.

Flash firmware over USB, erase flash, watch the serial console, and read or
push configuration over the USB CDC protocol. This is the developer-facing
counterpart to the end-user GUI configurator.

Examples:
    python tools/as_flash.py ports
    python tools/as_flash.py flash                 # flashes firmware/build
    python tools/as_flash.py flash --port COM7 --baud 921600
    python tools/as_flash.py erase --port COM7
    python tools/as_flash.py monitor --port COM7
    python tools/as_flash.py info
    python tools/as_flash.py config get -o myprofile.json
    python tools/as_flash.py config set -i myprofile.json --reboot

Flashing requires esptool (pip install -r tools/requirements.txt). OTA is not
wired yet; it lands with the as_web component.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time

# Allow `from alphastick import ...` no matter the current directory.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from alphastick import profile as profile_mod  # noqa: E402
from alphastick.protocol import (  # noqa: E402
    AlphaStick,
    DeviceError,
    find_port,
    list_candidate_ports,
)

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_BUILD_DIR = os.path.join(REPO_ROOT, "firmware", "build")
CHIP = "esp32s3"


def cmd_ports(_args: argparse.Namespace) -> int:
    ports = list_candidate_ports()
    if not ports:
        print("No serial ports found.")
        return 1
    print("Available serial ports (Alpha Stick listed first):")
    for device, desc in ports:
        print(f"  {desc}")
    return 0


def _esptool(args: list[str]) -> int:
    """Run esptool as a module so it uses the active Python environment."""
    cmd = [sys.executable, "-m", "esptool"] + args
    print("+ " + " ".join(cmd))
    try:
        return subprocess.call(cmd)
    except FileNotFoundError:
        print("esptool not found. Install it: pip install -r tools/requirements.txt")
        return 1


def cmd_flash(args: argparse.Namespace) -> int:
    build_dir = args.build_dir
    flasher_args_path = os.path.join(build_dir, "flasher_args.json")
    if not os.path.isfile(flasher_args_path):
        print(f"No flasher_args.json in {build_dir}.")
        print("Build first:  cd firmware && idf.py build")
        return 1

    with open(flasher_args_path, "r", encoding="utf-8") as fh:
        fa = json.load(fh)

    port = find_port(args.port)
    if not port:
        print("No device port found; pass --port (see: as_flash.py ports).")
        return 1

    write_args = list(fa.get("write_flash_args", []))
    flash_files = fa.get("flash_files", {})
    if not flash_files:
        print("flasher_args.json has no flash_files; unexpected build output.")
        return 1

    offsets_and_files: list[str] = []
    for offset, rel in sorted(flash_files.items(), key=lambda kv: int(kv[0], 16)):
        path = os.path.join(build_dir, rel)
        if not os.path.isfile(path):
            print(f"Missing image: {path}")
            return 1
        offsets_and_files += [offset, path]

    esp_args = ["--chip", CHIP, "-p", port, "-b", str(args.baud),
                "write_flash"] + write_args + offsets_and_files
    rc = _esptool(esp_args)
    if rc == 0:
        print("\nFlash complete. Power-cycle if it does not enumerate.")
    return rc


def cmd_erase(args: argparse.Namespace) -> int:
    port = find_port(args.port)
    if not port:
        print("No device port found; pass --port.")
        return 1
    return _esptool(["--chip", CHIP, "-p", port, "erase_flash"])


def cmd_monitor(args: argparse.Namespace) -> int:
    import serial  # local import: only needed for monitor

    port = args.port or find_port()
    if not port:
        print("No serial port found; pass --port (see: as_flash.py ports).")
        return 1
    print(f"Monitoring {port} at {args.baud} baud. Ctrl-C to stop.")
    try:
        with serial.Serial(port, baudrate=args.baud, timeout=0.2) as ser:
            while True:
                data = ser.readline()
                if data:
                    sys.stdout.write(data.decode("utf-8", errors="replace"))
                    sys.stdout.flush()
    except KeyboardInterrupt:
        print("\nStopped.")
        return 0
    except serial.SerialException as exc:
        print(f"Serial error: {exc}")
        return 1


def cmd_info(args: argparse.Namespace) -> int:
    try:
        with AlphaStick(args.port) as dev:
            info = dev.info()
    except DeviceError as exc:
        print(f"Error: {exc}")
        return 1
    print(f"Device:   {info.get('device')}")
    print(f"Firmware: {info.get('fw')}")
    print(f"Protocol: {info.get('proto')}")
    print(f"Profile:  {info.get('profile_name')}")
    return 0


def cmd_config(args: argparse.Namespace) -> int:
    try:
        with AlphaStick(args.port) as dev:
            if args.action == "get":
                prof = profile_mod.merge_defaults(dev.get_profile())
                text = json.dumps(prof, indent=2)
                if args.output:
                    with open(args.output, "w", encoding="utf-8") as fh:
                        fh.write(text + "\n")
                    print(f"Wrote {args.output}")
                else:
                    print(text)
            elif args.action == "set":
                prof = profile_mod.load(args.input)
                problems = profile_mod.validate(prof)
                if problems:
                    print("Profile has problems:")
                    for p in problems:
                        print(f"  - {p}")
                    return 1
                dev.set_profile(prof)
                print("Profile applied.")
                if args.reboot:
                    dev.reboot()
                    print("Reboot requested.")
    except (DeviceError, OSError, ValueError) as exc:
        print(f"Error: {exc}")
        return 1
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)

    p_ports = sub.add_parser("ports", help="list serial ports")
    p_ports.set_defaults(func=cmd_ports)

    p_flash = sub.add_parser("flash", help="flash firmware over USB")
    p_flash.add_argument("--port", help="serial port (auto-detect if omitted)")
    p_flash.add_argument("--baud", type=int, default=460800, help="flash baud rate")
    p_flash.add_argument("--build-dir", default=DEFAULT_BUILD_DIR,
                         help="IDF build directory (default: firmware/build)")
    p_flash.set_defaults(func=cmd_flash)

    p_erase = sub.add_parser("erase", help="erase the whole flash")
    p_erase.add_argument("--port", help="serial port")
    p_erase.set_defaults(func=cmd_erase)

    p_mon = sub.add_parser("monitor", help="print the serial console")
    p_mon.add_argument("--port", help="serial port")
    p_mon.add_argument("--baud", type=int, default=115200, help="console baud rate")
    p_mon.set_defaults(func=cmd_monitor)

    p_info = sub.add_parser("info", help="read device info over USB CDC")
    p_info.add_argument("--port", help="serial port")
    p_info.set_defaults(func=cmd_info)

    p_cfg = sub.add_parser("config", help="read or push a profile over USB CDC")
    p_cfg.add_argument("action", choices=["get", "set"])
    p_cfg.add_argument("--port", help="serial port")
    p_cfg.add_argument("-o", "--output", help="write profile JSON here (get)")
    p_cfg.add_argument("-i", "--input", help="profile JSON to push (set)")
    p_cfg.add_argument("--reboot", action="store_true", help="reboot after set")
    p_cfg.set_defaults(func=cmd_config)

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    if getattr(args, "command", None) == "config" and args.action == "set" and not args.input:
        parser.error("config set needs -i/--input")
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
