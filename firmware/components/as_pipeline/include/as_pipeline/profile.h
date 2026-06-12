#pragma once

#include <cstdint>

namespace as {

enum class Mode : uint8_t {
    GAMEPAD = 0,
    MOUSE,
    KEYBOARD,
    DUAL,   // gamepad + mouse simultaneously
    ATOS,   // AS-Link out, HID silent
};

enum class CurveType : uint8_t {
    LINEAR = 0,
    EXPO,
    LOG,
    SCURVE,
};

// Mirrors the JSON schema in docs/CONFIGURATION.md. Persisted as an NVS blob;
// bump CONFIG_BLOB_VERSION in as_config when this struct changes.
struct Profile {
    char name[32] = "default";
    Mode mode = Mode::GAMEPAD;

    // mount
    float rotation_deg = 0.0f;
    float gravity_tare_x = 0.0f;
    float gravity_tare_y = 0.0f;
    bool invert_x = false;
    bool invert_y = false;

    // radial deadzone
    float deadzone_inner = 0.04f;
    float deadzone_outer = 0.98f;

    // curve
    CurveType curve = CurveType::EXPO;
    float curve_gain = 1.0f;
    float curve_expo = 0.4f;

    // 1-euro filter; min_cutoff <= 0 disables
    float oe_min_cutoff = 1.0f;
    float oe_beta = 0.02f;

    // mouse
    float mouse_max_px_s = 900.0f;
};

}  // namespace as
