#pragma once

namespace as {

// NimBLE HID service exposing the same three report IDs as USB
// (docs/FIRMWARE.md). Phase 0 spike; stub until then so the build stays light.
void ble_hid_init();

}  // namespace as
