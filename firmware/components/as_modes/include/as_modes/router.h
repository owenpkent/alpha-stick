#pragma once

#include "as_sensing/mailbox.h"
#include "as_sensing/stick_sample.h"

namespace as {

// Starts the router task (core 0): consumes the sensing mailbox at 1 kHz and
// maps the active mode onto the HID writers. Gamepad and mouse paths work;
// keyboard, dual refinements, and AS-Link output are tracked TODOs.
void router_start(Mailbox<StickSample> &mailbox);

}  // namespace as
