# Alpha Stick host tools

Two native, cross-platform tools that talk to an Alpha Stick V2 over USB:

- **`configurator.py`** (end user): a GUI to change axes, deadzone, response
  curve, tremor filter, mouse speed, button mappings, and macros.
- **`as_flash.py`** (developer): a CLI to flash firmware, erase, watch the
  serial console, and read or push profiles for scripting.

Both share `alphastick/`, a small library wrapping the USB CDC config protocol
and the profile model.

## Install

Requires Python 3.9+ (with `tkinter` for the GUI, which ships with the
python.org and most distro builds).

```powershell
python -m pip install -r tools/requirements.txt
```

## End-user configurator

```powershell
python tools/configurator.py
```

1. Pick the **Port** (the Alpha Stick is listed first) and **Connect**. The app
   reads the device's current profile.
2. Edit on the tabs. **Stick** and **Mouse** changes are pushed with
   **Apply (live)** and take effect immediately (no reboot).
3. **Save on device** persists across reboots. **Save file...** writes a `.json`
   profile you can share or back up; **Open file...** loads one.

Button mappings and macros are authored and saved now; the firmware applies them
once the input subsystem (jacks, sip/puff) and macro engine land. See
[scope note](#scope) below.

## Developer flasher

```powershell
# See available ports
python tools/as_flash.py ports

# Build the firmware first (ESP-IDF), then flash firmware/build
cd firmware; idf.py build; cd ..
python tools/as_flash.py flash --port COM7 --baud 921600

# Other actions
python tools/as_flash.py erase   --port COM7
python tools/as_flash.py monitor --port COM7
python tools/as_flash.py info
python tools/as_flash.py config get -o myprofile.json
python tools/as_flash.py config set -i myprofile.json --reboot
```

`flash` reads `firmware/build/flasher_args.json` (produced by `idf.py build`)
for the image offsets, so it always matches the partition layout. OTA is not
wired yet; it arrives with the `as_web` component.

## USB CDC config protocol

The device exposes a newline-delimited JSON command channel on its USB CDC
interface (firmware: `components/as_config/config_protocol.cpp`). Each request
is one JSON object; each response is one JSON line with an `ok` boolean.

| Command | Request | Response |
|---------|---------|----------|
| info | `{"cmd":"info"}` | `{"ok":true,"device":...,"fw":...,"proto":"1","profile_name":...}` |
| get | `{"cmd":"get"}` | `{"ok":true,"profile":{...}}` |
| set | `{"cmd":"set","profile":{...}}` | `{"ok":true}` (merges named keys, applies live) |
| save | `{"cmd":"save"}` | `{"ok":true}` |
| defaults | `{"cmd":"defaults"}` | `{"ok":true,"profile":{...}}` |
| reboot | `{"cmd":"reboot"}` | `{"ok":true}` then the device restarts |

Errors return `{"ok":false,"error":"..."}`. The firmware ignores profile keys it
does not implement, so a host can always send a full profile (including
`buttons` and `macros`) and the device applies the subset it supports.

The profile JSON schema is documented in
[docs/CONFIGURATION.md](../docs/CONFIGURATION.md).

## Scope

The firmware implements the stick/mouse subset of the profile today (mode,
mount, deadzone, curve, filter, mouse). `buttons` and `macros` are part of the
host profile model and round-trip through files now; their runtime in firmware
is blocked on the jack/sip-puff input subsystem and is tracked in
[TODO.md](../TODO.md).
