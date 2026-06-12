#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
namespace as {
extern "C" {
#endif

// TinyUSB composite device: one HID interface carrying gamepad (report ID 1,
// 16-bit axes), mouse (ID 2), keyboard (ID 3), plus a CDC config channel.
// Mode switches change which reports stream; the device never re-enumerates.

void usb_hid_init(void);
bool usb_hid_ready(void);

bool usb_hid_send_gamepad(int16_t x, int16_t y, uint8_t z, uint16_t buttons);
bool usb_hid_send_mouse(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel);

#ifdef __cplusplus
}  // extern "C"
}  // namespace as
#endif
