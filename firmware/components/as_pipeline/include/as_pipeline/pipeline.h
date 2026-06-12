#pragma once

#include "as_pipeline/one_euro.h"
#include "as_pipeline/profile.h"

namespace as {

// Per-axis values in, normalized [-1, +1] out. Stage order is load-bearing
// and documented in docs/FIRMWARE.md: tare -> rotate/invert -> radial
// deadzone -> curve -> 1-euro. Calibration map and notch live upstream and
// later respectively (TODO).
class Pipeline {
public:
    void set_profile(const Profile &p);

    struct Out {
        float x;
        float y;
        float z;
    };

    Out process(float x, float y, float z, float dt_s);

private:
    Profile profile_{};
    OneEuro fx_;
    OneEuro fy_;
    float rot_cos_ = 1.0f;
    float rot_sin_ = 0.0f;
};

}  // namespace as
