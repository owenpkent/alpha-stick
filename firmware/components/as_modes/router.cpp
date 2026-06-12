#include "as_modes/router.h"

#include <algorithm>
#include <cmath>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "as_config/config.h"
#include "as_hid_usb/usb_hid.h"

namespace as {

namespace {

const char *TAG = "router";

int16_t to_i16(float v)
{
    return (int16_t)std::lround(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
}

struct MouseAccum {
    float x = 0.0f;
    float y = 0.0f;

    int8_t take(float &acc)
    {
        const float clipped = std::clamp(acc, -127.0f, 127.0f);
        const int8_t out = (int8_t)clipped;
        acc -= out;
        return out;
    }
};

void router_task(void *arg)
{
    auto &mailbox = *static_cast<Mailbox<StickSample> *>(arg);
    MouseAccum accum;
    bool z_latched = false;
    bool logged_silent_mode = false;

    TickType_t wake = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&wake, 1);  // 1 ms

        StickSample s;
        if (!mailbox.read(s)) {
            continue;
        }
        const Profile &p = config_profile();

        // Z-press click with hysteresis: press above 0.6, release below 0.4.
        if (!z_latched && s.z > 0.6f) {
            z_latched = true;
        } else if (z_latched && s.z < 0.4f) {
            z_latched = false;
        }

        if (p.mode == Mode::ATOS || p.mode == Mode::KEYBOARD) {
            if (!logged_silent_mode) {
                ESP_LOGW(TAG, "mode %d not implemented yet, HID silent", (int)p.mode);
                logged_silent_mode = true;
            }
            continue;
        }

        if (!usb_hid_ready()) {
            continue;
        }

        const bool gamepad = (p.mode == Mode::GAMEPAD || p.mode == Mode::DUAL);
        const bool mouse = (p.mode == Mode::MOUSE || p.mode == Mode::DUAL);

        if (gamepad) {
            const uint16_t buttons = z_latched ? 0x0001 : 0x0000;
            const uint8_t z8 = (uint8_t)std::lround(std::clamp(s.z, 0.0f, 1.0f) * 255.0f);
            usb_hid_send_gamepad(to_i16(s.x), to_i16(s.y), z8, buttons);
        }

        if (mouse) {
            // Velocity mode: deflection -> px/s, fractional px accumulated so
            // slow drifts are not lost to int8 truncation.
            accum.x += s.x * p.mouse_max_px_s * 0.001f;
            accum.y += s.y * p.mouse_max_px_s * 0.001f;
            const int8_t dx = accum.take(accum.x);
            const int8_t dy = accum.take(accum.y);
            const uint8_t buttons = (!gamepad && z_latched) ? 0x01 : 0x00;  // left
            if (dx != 0 || dy != 0 || buttons != 0) {
                usb_hid_send_mouse(buttons, dx, dy, 0);
            }
        }
    }
}

}  // namespace

void router_start(Mailbox<StickSample> &mailbox)
{
    xTaskCreatePinnedToCore(router_task, "as_router", 4096, &mailbox,
                            configMAX_PRIORITIES - 6, nullptr, 0);
}

}  // namespace as
