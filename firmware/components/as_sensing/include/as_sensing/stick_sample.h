#pragma once

#include <cstdint>

namespace as {

// One processed stick sample, the unit of exchange between the 1 kHz sensing
// task and every output writer (docs/FIRMWARE.md).
struct StickSample {
    float x = 0.0f;            // normalized [-1, +1], post-pipeline
    float y = 0.0f;
    float z = 0.0f;            // press axis [0, +1]
    uint32_t t_us = 0;         // sample timestamp (esp_timer)
    bool cal_valid = false;    // calibration map loaded and field in envelope
    bool dual_ok = false;      // both sensors agree within threshold
    bool tare_stable = false;  // center reference trustworthy
    bool simulated = false;    // produced by the no-sensor simulator
};

}  // namespace as
