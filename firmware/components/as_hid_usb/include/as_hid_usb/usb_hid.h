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

// USB CDC config channel. The driver buffers received bytes into newline-
// delimited lines and hands each (without the newline) to the registered
// handler; usb_hid_cdc_write sends a response back on the same channel.
typedef void (*as_cdc_line_cb_t)(const char *line);
void usb_hid_set_cdc_line_handler(as_cdc_line_cb_t cb);
void usb_hid_cdc_write(const char *data, unsigned len);

#ifdef __cplusplus
}  // extern "C"
}  // namespace as
#endif
