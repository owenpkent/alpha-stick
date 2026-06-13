#include "as_sensing/sensing.h"

#include <cmath>

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "as_config/config.h"
#include "as_pipeline/pipeline.h"
#include "as_sensing/tmag5273.h"

namespace as {

namespace {

const char *TAG = "sensing";

constexpr gpio_num_t kSda = GPIO_NUM_8;
constexpr gpio_num_t kScl = GPIO_NUM_9;
constexpr float kFullScaleRad = 5.0f * (float)M_PI / 180.0f;  // +/-5 deg throw

struct Ctx {
    Mailbox<StickSample> *mailbox;
    Tmag5273 sensor;
    bool live = false;
};

// Direction-based tilt estimate: angle of the field vector against the sensor
// normal, which is what makes the measurement gap- and temperature-tolerant
// (docs/DESIGN_V2.md sec 4). Placeholder until the calibration map exists:
// assumes the rest field is along +Z and axes are aligned with the PCB.
void field_to_norm(float bx, float by, float bz, float &nx, float &ny, float &nz,
                   float baseline_mag)
{
    const float az = std::fabs(bz) > 1e-3f ? bz : 1e-3f;
    nx = std::atan2(bx, az) / kFullScaleRad;
    ny = std::atan2(by, az) / kFullScaleRad;

    // Z press: |B| rises as the gap closes. Gain 8 maps a ~12% magnitude
    // increase to full press; tune on the bench.
    const float mag = std::sqrt(bx * bx + by * by + bz * bz);
    nz = baseline_mag > 1e-3f ? (mag / baseline_mag - 1.0f) * 8.0f : 0.0f;
}

void sensing_task(void *arg)
{
    Ctx *ctx = static_cast<Ctx *>(arg);
    Pipeline pipeline;
    pipeline.set_profile(config_profile());
    uint32_t cfg_gen = config_generation();

    float baseline_mag = 0.0f;
    if (ctx->live) {
        // Capture the rest-field magnitude as the Z-press baseline.
        float acc = 0.0f;
        int n = 0;
        for (int i = 0; i < 200; ++i) {
            float bx, by, bz;
            if (ctx->sensor.read_mT(bx, by, bz) == ESP_OK) {
                acc += std::sqrt(bx * bx + by * by + bz * bz);
                ++n;
            }
            vTaskDelay(1);
        }
        baseline_mag = n > 0 ? acc / n : 0.0f;
        ESP_LOGI(TAG, "LIVE: TMAG5273 at 0x%02X, |B| baseline %.2f mT",
                 ctx->sensor.address(), baseline_mag);
    } else {
        ESP_LOGW(TAG, "SIMULATION: no sensor found, synthesizing input");
    }

    TickType_t wake = xTaskGetTickCount();
    uint32_t prev_us = (uint32_t)esp_timer_get_time();
    for (;;) {
        vTaskDelayUntil(&wake, 1);  // 1 ms with CONFIG_FREERTOS_HZ=1000

        // Pick up config edits pushed over USB CDC without a reboot.
        const uint32_t gen = config_generation();
        if (gen != cfg_gen) {
            pipeline.set_profile(config_profile());
            cfg_gen = gen;
        }

        const uint32_t now_us = (uint32_t)esp_timer_get_time();
        const float dt_s = (now_us - prev_us) * 1e-6f;
        prev_us = now_us;

        float nx, ny, nz;
        bool cal_valid;
        if (ctx->live) {
            float bx, by, bz;
            if (ctx->sensor.read_mT(bx, by, bz) != ESP_OK) {
                continue;  // missed sample; watchdog wiring is a TODO
            }
            field_to_norm(bx, by, bz, nx, ny, nz, baseline_mag);
            cal_valid = false;  // true once the real calibration map lands
        } else {
            // Slow circle at 0.2 Hz, 80% deflection, Z pulse every 3 s.
            const float t = now_us * 1e-6f;
            const float w = 2.0f * (float)M_PI * 0.2f;
            nx = 0.8f * std::sin(w * t);
            ny = 0.8f * std::cos(w * t);
            nz = (std::fmod(t, 3.0f) < 0.2f) ? 1.0f : 0.0f;
            cal_valid = true;
        }

        const Pipeline::Out o = pipeline.process(nx, ny, nz, dt_s);

        StickSample s;
        s.x = o.x;
        s.y = o.y;
        s.z = o.z;
        s.t_us = now_us;
        s.cal_valid = cal_valid;
        s.dual_ok = false;  // sensor B on I2C1 + cross-check is a Phase 0 task
        s.tare_stable = true;
        s.simulated = !ctx->live;
        ctx->mailbox->write(s);
    }
}

}  // namespace

void sensing_start(Mailbox<StickSample> &mailbox)
{
    static Ctx ctx;
    ctx.mailbox = &mailbox;

    i2c_master_bus_config_t bus_cfg = {};
    bus_cfg.i2c_port = I2C_NUM_0;
    bus_cfg.sda_io_num = kSda;
    bus_cfg.scl_io_num = kScl;
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.glitch_ignore_cnt = 7;
    bus_cfg.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus = nullptr;
    if (i2c_new_master_bus(&bus_cfg, &bus) == ESP_OK &&
        ctx.sensor.probe(bus) == ESP_OK &&
        ctx.sensor.start_continuous() == ESP_OK) {
        ctx.live = true;
    }

#if !CONFIG_AS_SIMULATE_WHEN_NO_SENSOR
    if (!ctx.live) {
        ESP_LOGE(TAG, "no sensor and simulation disabled; sensing not started");
        return;
    }
#endif

    xTaskCreatePinnedToCore(sensing_task, "as_sensing", 4096, &ctx,
                            configMAX_PRIORITIES - 3, nullptr, 1);
}

}  // namespace as
