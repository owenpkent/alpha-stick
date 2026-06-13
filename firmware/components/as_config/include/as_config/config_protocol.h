#pragma once

namespace as {

// Handle one line of the USB CDC config protocol (a single JSON object). The
// transport (as_hid_usb) buffers bytes into newline-delimited lines and calls
// this for each. Returns a malloc'd JSON response line WITHOUT a trailing
// newline, or nullptr for a blank/ignored line. The caller must free() it and
// is responsible for appending the newline on the wire.
//
// Commands (field "cmd"):
//   info     -> device id, firmware version, protocol version, active profile
//   get      -> the active profile as JSON
//   set      -> apply+persist the "profile" object (partial objects merge)
//   defaults -> reset to firmware defaults, returns the profile
//   save     -> ack (set already persists; kept for an explicit-save UX)
//   reboot   -> ack, then the device restarts shortly after (see take_reboot)
char *config_protocol_handle_line(const char *line);

// True once if a reboot command was seen since the last call. The transport
// checks this after sending the response so the ack reaches the host before
// the restart.
bool config_protocol_take_reboot();

}  // namespace as
