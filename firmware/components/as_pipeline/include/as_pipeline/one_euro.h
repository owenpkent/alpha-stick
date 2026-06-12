#pragma once

namespace as {

// 1-euro filter (Casiez et al.): an adaptive low-pass whose cutoff rises with
// input speed, so rest tremor is smoothed without lagging deliberate motion.
class OneEuro {
public:
    void configure(float min_cutoff, float beta);
    void reset();
    float filter(float x, float dt_s);

private:
    static float alpha(float cutoff, float dt_s);

    float min_cutoff_ = 1.0f;
    float beta_ = 0.02f;
    float d_cutoff_ = 1.0f;
    bool primed_ = false;
    float x_prev_ = 0.0f;
    float dx_prev_ = 0.0f;
};

}  // namespace as
