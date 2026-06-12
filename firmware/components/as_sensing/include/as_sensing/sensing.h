#pragma once

#include "as_sensing/mailbox.h"
#include "as_sensing/stick_sample.h"

namespace as {

// Starts the 1 kHz sensing task (core 1, highest app priority). Probes for a
// TMAG5273 on I2C0 (SDA 8 / SCL 9); falls back to the simulator when enabled
// and no sensor answers.
void sensing_start(Mailbox<StickSample> &mailbox);

}  // namespace as
