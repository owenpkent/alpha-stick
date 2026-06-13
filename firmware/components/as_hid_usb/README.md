# as_hid_usb

TinyUSB composite USB device for Alpha Stick V2. Presents one HID interface that
carries three report IDs (gamepad, mouse, keyboard) plus a CDC ACM serial channel
for configuration. Mode switches change *which* reports stream; the descriptors are
constant, so the device never re-enumerates and the OS sees one stable device.

Architecture context is in [docs/FIRMWARE.md](../../../docs/FIRMWARE.md) (USB and BLE
HID section); the 16-bit report rationale is DESIGN_V2 requirement R8.

---

## USB layout

Composite device with an Interface Association Descriptor (CDC requires IAD), so the
device class is `MISC / COMMON / IAD`.

| Interface | Num | Endpoint(s) | Purpose |
|-----------|-----|-------------|---------|
| HID | 0 | `0x81` IN | gamepad + mouse + keyboard reports |
| CDC notify | 1 | `0x82` IN | CDC ACM management |
| CDC data | 2 | `0x03` OUT, `0x83` IN | config channel bytes |

### Device descriptor

| Field | Value | Notes |
|-------|-------|-------|
| idVendor | `0x303A` | Espressif's VID |
| idProduct | `0x4001` | **placeholder**, request a pid.codes PID before any public release |
| bcdDevice | `0x0200` | tracks hardware V2 |
| bcdUSB | `0x0200` | USB 2.0 full speed |

String descriptors: manufacturer "Alpha Stick Project", product "Alpha Stick",
serial "AS2-0001" (placeholder), HID interface "Alpha Stick HID", config interface
"Alpha Stick Config".

---

## Reports

### Report ID 1: gamepad (custom descriptor)

7-byte payload, little-endian, packed:

| Offset | Field | Type | Range | Maps to |
|--------|-------|------|-------|---------|
| 0 | X | int16 | -32767..32767 | stick X, post-pipeline |
| 2 | Y | int16 | -32767..32767 | stick Y |
| 4 | Z | uint8 | 0..255 | press axis |
| 5 | buttons | uint16 | bitfield, 16 buttons | Z-click, jacks, sip/puff |

16-bit axes are deliberate: an 8-bit stick quantizes the usable centre too coarsely
for tremor-filtered fine control (DESIGN_V2 R8). On the wire the report is 8 bytes
(1 report ID + 7 payload).

### Report ID 2: mouse

Stock `TUD_HID_REPORT_DESC_MOUSE`: 5 buttons, **int8** dx/dy, int8 wheel, int8 pan.
Sent via `tud_hid_n_mouse_report`.

### Report ID 3: keyboard

Stock `TUD_HID_REPORT_DESC_KEYBOARD` (standard 6KRO). The descriptor and report ID
are present, but there is no `usb_hid_send_keyboard` writer yet (see Known gaps).

---

## CDC config channel

`cdc_rx_callback` currently drains received bytes and logs the count. The JSON-lines
config protocol (WebSerial-compatible, shared with the web UI) is not implemented yet;
this is the hook for it.

---

## Public API

```c
#include "as_hid_usb/usb_hid.h"

void usb_hid_init(void);   // install TinyUSB driver + CDC ACM; ESP_ERROR_CHECK on failure
bool usb_hid_ready(void);  // tud_mounted() && HID instance 0 ready

bool usb_hid_send_gamepad(int16_t x, int16_t y, uint8_t z, uint16_t buttons);
bool usb_hid_send_mouse(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel);
```

The send functions return the TinyUSB queue result (false if the endpoint is busy or
the device is not mounted). The router task is expected to gate on `usb_hid_ready()`
and drop, not block, when USB is not the active sink.

All three report IDs share one HID IN endpoint, so only one report fits per host poll.
A caller submitting two reports back to back (gamepad then mouse in DUAL mode) will see
the second return false, because the first left the endpoint busy. The router handles
this by alternating gamepad and mouse across ticks (`as_modes/router.cpp`); any new
caller must serialize the same way rather than submit multiple reports per poll.

The header wraps the API in `namespace as { extern "C" { ... } }` under C++, so it is
callable from both the C implementation and the C++ router.

### TinyUSB callbacks implemented here

- `tud_hid_descriptor_report_cb` returns the combined report descriptor.
- `tud_hid_get_report_cb` answers INPUT probes with a zeroed report of the right
  length (gamepad 7, mouse/keyboard via the stock structs); other types return 0.
- `tud_hid_set_report_cb` is a stub; keyboard LED output reports would land here.

---

## Dependencies

From `idf_component.yml`: ESP-IDF >= 5.1 and `espressif/esp_tinyusb ^1`. Requires the
ESP32-S3 native USB OTG peripheral (HARDWARE.md: D- = GPIO19, D+ = GPIO20).

---

## Known gaps (Phase 0)

- **VERIFY** the `tinyusb_config_t` field names against the installed `esp_tinyusb ^1`
  at build time; the init code carries a note that the component API may have drifted.
- No keyboard report writer yet, though the keyboard descriptor is enumerated.
- The mouse descriptor is 8-bit relative (stock TinyUSB) and the gamepad has no hat.
  FIRMWARE.md now matches this. Richer shapes (int16 mouse deltas, a gamepad hat) are
  possible future enrichments, not current behaviour; this README is the wire-format
  source of truth.
- CDC config protocol is a logging stub (see above).
- Confirm Windows and the Xbox Adaptive Controller both enumerate the composite device
  cleanly and that runtime mode switches never trigger re-enumeration.
