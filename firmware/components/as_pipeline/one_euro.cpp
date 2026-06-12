#include "as_pipeline/one_euro.h"

#include <cmath>

namespace as {

void OneEuro::configure(float min_cutoff, float beta)
{
    min_cutoff_ = min_cutoff;
    beta_ = beta;
    reset();
}

void OneEuro::reset()
{
    primed_ = false;
    x_prev_ = 0.0f;
    dx_prev_ = 0.0f;
}

float OneEuro::alpha(float cutoff, float dt_s)
{
    const float tau = 1.0f / (2.0f * (float)M_PI * cutoff);
    return 1.0f / (1.0f + tau / dt_s);
}

float OneEuro::filter(float x, float dt_s)
{
    if (min_cutoff_ <= 0.0f) {
        return x;  // disabled
    }
    if (!primed_ || dt_s <= 0.0f) {
        primed_ = true;
        x_prev_ = x;
        dx_prev_ = 0.0f;
        return x;
    }

    const float dx = (x - x_prev_) / dt_s;
    const float a_d = alpha(d_cutoff_, dt_s);
    const float dx_hat = a_d * dx + (1.0f - a_d) * dx_prev_;

    const float cutoff = min_cutoff_ + beta_ * std::fabs(dx_hat);
    const float a = alpha(cutoff, dt_s);
    const float x_hat = a * x + (1.0f - a) * x_prev_;

    x_prev_ = x_hat;
    dx_prev_ = dx_hat;
    return x_hat;
}

}  // namespace as
