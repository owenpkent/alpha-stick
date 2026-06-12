#include "as_pipeline/pipeline.h"

#include <algorithm>
#include <cmath>

namespace as {

namespace {

float clamp1(float v)
{
    return std::clamp(v, -1.0f, 1.0f);
}

// RC-style expo: blends linear into cubic; expo=0 is linear, expo=1 is x^3.
float apply_curve(float m, CurveType type, float gain, float expo)
{
    float out;
    switch (type) {
    case CurveType::EXPO:
        out = m * (1.0f - expo) + m * m * m * expo;
        break;
    case CurveType::LOG:
        out = std::sqrt(m);
        break;
    case CurveType::SCURVE:
        out = m * m * (3.0f - 2.0f * m);  // smoothstep
        break;
    case CurveType::LINEAR:
    default:
        out = m;
        break;
    }
    return std::clamp(out * gain, 0.0f, 1.0f);
}

}  // namespace

void Pipeline::set_profile(const Profile &p)
{
    profile_ = p;
    const float rad = p.rotation_deg * (float)M_PI / 180.0f;
    rot_cos_ = std::cos(rad);
    rot_sin_ = std::sin(rad);
    fx_.configure(p.oe_min_cutoff, p.oe_beta);
    fy_.configure(p.oe_min_cutoff, p.oe_beta);
}

Pipeline::Out Pipeline::process(float x, float y, float z, float dt_s)
{
    // 1. mount tare
    x -= profile_.gravity_tare_x;
    y -= profile_.gravity_tare_y;

    // 2. rotation + invert
    float rx = x * rot_cos_ - y * rot_sin_;
    float ry = x * rot_sin_ + y * rot_cos_;
    if (profile_.invert_x) rx = -rx;
    if (profile_.invert_y) ry = -ry;

    // 3. radial deadzone with rescale (preserves diagonals)
    const float mag = std::sqrt(rx * rx + ry * ry);
    float out_mag = 0.0f;
    if (mag > profile_.deadzone_inner) {
        const float span = profile_.deadzone_outer - profile_.deadzone_inner;
        const float t = std::min((mag - profile_.deadzone_inner) / std::max(span, 1e-6f), 1.0f);
        // 4. curve on magnitude
        out_mag = apply_curve(t, profile_.curve, profile_.curve_gain, profile_.curve_expo);
    }
    const float scale = (mag > 1e-6f) ? out_mag / mag : 0.0f;

    // 5. 1-euro filter per axis
    Out o;
    o.x = clamp1(fx_.filter(rx * scale, dt_s));
    o.y = clamp1(fy_.filter(ry * scale, dt_s));
    o.z = std::clamp(z, 0.0f, 1.0f);
    return o;
}

}  // namespace as
