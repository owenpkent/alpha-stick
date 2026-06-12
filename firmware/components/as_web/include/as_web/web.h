#pragma once

namespace as {

// WiFi AP + web configurator + OTA endpoints (config mode only). Stub; the
// CDC/WebSerial JSON channel in as_hid_usb is the nearer-term config path.
void web_init();

}  // namespace as
