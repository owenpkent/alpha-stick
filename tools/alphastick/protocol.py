"""USB CDC config-protocol client for Alpha Stick V2.

The device exposes a newline-delimited JSON command channel on its USB CDC
interface (firmware: components/as_config/config_protocol.cpp). Each request is
one JSON object with a "cmd" field; each response is one JSON object with an
"ok" boolean. This module wraps the serial transport and the request/response
framing so both the GUI and the CLI can speak it.
"""

from __future__ import annotations

import json
from typing import Any, Optional

try:
    import serial
    from serial.tools import list_ports
except ImportError as exc:  # pragma: no cover - dependency hint
    raise SystemExit(
        "pyserial is required. Install it with: pip install -r tools/requirements.txt"
    ) from exc


# Espressif's VID and the (placeholder) Alpha Stick PID, from the USB device
# descriptor in firmware/components/as_hid_usb/usb_hid.c.
ALPHA_VID = 0x303A
ALPHA_PID = 0x4001


class DeviceError(Exception):
    """A protocol-level or transport-level failure talking to the device."""


def list_candidate_ports() -> list[tuple[str, str]]:
    """Return (device, description) for every serial port, Alpha Stick first."""
    ports = list(list_ports.comports())
    ports.sort(key=lambda p: (p.vid != ALPHA_VID or p.pid != ALPHA_PID, p.device))
    return [(p.device, _describe(p)) for p in ports]


def find_port(preferred: Optional[str] = None) -> Optional[str]:
    """Pick a serial port: the preferred one if present, else by VID/PID."""
    ports = list(list_ports.comports())
    if preferred:
        for p in ports:
            if p.device == preferred:
                return p.device
        return preferred  # trust the caller even if enumeration missed it
    for p in ports:
        if p.vid == ALPHA_VID and p.pid == ALPHA_PID:
            return p.device
    return None


def _describe(port) -> str:
    if port.vid == ALPHA_VID and port.pid == ALPHA_PID:
        return f"{port.device} (Alpha Stick)"
    desc = port.description or "serial port"
    return f"{port.device} ({desc})"


class AlphaStick:
    """Open connection to an Alpha Stick over its USB CDC config channel."""

    def __init__(self, port: Optional[str] = None, timeout: float = 2.0):
        self.port_name = find_port(port)
        if not self.port_name:
            raise DeviceError(
                "Alpha Stick not found. No USB CDC port with VID:PID "
                f"{ALPHA_VID:04X}:{ALPHA_PID:04X}. Pass an explicit --port, or "
                "check the cable and that the device is in normal (not download) mode."
            )
        # Baud is ignored by USB CDC; the timeout governs reads.
        self.ser = serial.Serial(self.port_name, baudrate=115200, timeout=timeout)

    def close(self) -> None:
        try:
            self.ser.close()
        except Exception:
            pass

    def __enter__(self) -> "AlphaStick":
        return self

    def __exit__(self, *_exc: object) -> None:
        self.close()

    def _command(self, obj: dict[str, Any]) -> dict[str, Any]:
        line = (json.dumps(obj) + "\n").encode("utf-8")
        self.ser.reset_input_buffer()
        self.ser.write(line)
        self.ser.flush()
        raw = self.ser.readline()
        if not raw:
            raise DeviceError(
                "No response (timeout). The device may be in download mode, or "
                "another program may hold the port."
            )
        try:
            resp = json.loads(raw.decode("utf-8", errors="replace").strip())
        except json.JSONDecodeError as exc:
            raise DeviceError(f"Malformed response: {raw!r}") from exc
        if not isinstance(resp, dict) or not resp.get("ok", False):
            msg = resp.get("error", "device reported an error") if isinstance(resp, dict) else "bad response"
            raise DeviceError(str(msg))
        return resp

    # --- commands ---------------------------------------------------------

    def info(self) -> dict[str, Any]:
        return self._command({"cmd": "info"})

    def get_profile(self) -> dict[str, Any]:
        return self._command({"cmd": "get"})["profile"]

    def set_profile(self, profile: dict[str, Any]) -> None:
        # The firmware merges named keys and ignores ones it does not implement
        # (host-side buttons/macros), so the whole profile can be sent as-is.
        self._command({"cmd": "set", "profile": profile})

    def save(self) -> None:
        self._command({"cmd": "save"})

    def reset_defaults(self) -> dict[str, Any]:
        return self._command({"cmd": "defaults"})["profile"]

    def reboot(self) -> None:
        # The device may reset before acking; treat a timeout as success.
        try:
            self._command({"cmd": "reboot"})
        except DeviceError:
            pass
