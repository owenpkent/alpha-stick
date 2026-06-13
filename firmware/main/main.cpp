#include <cstdlib>
#include <cstring>

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "as_config/config.h"
#include "as_config/config_protocol.h"
#include "as_hid_ble/ble_hid.h"
#include "as_hid_usb/usb_hid.h"
#include "as_modes/router.h"
#include "as_sensing/mailbox.h"
#include "as_sensing/sensing.h"
#include "as_web/web.h"

static const char *TAG = "main";

static as::Mailbox<as::StickSample> s_mailbox;

// Bridge USB CDC config lines to the JSON protocol. Runs in the TinyUSB task
// context; the response is written back on the same CDC channel.
extern "C" void as_cdc_on_line(const char *line)
{
    char *resp = as::config_protocol_handle_line(line);
    if (resp) {
        as::usb_hid_cdc_write(resp, (unsigned)strlen(resp));
        as::usb_hid_cdc_write("\n", 1);
        free(resp);
    }
    if (as::config_protocol_take_reboot()) {
        vTaskDelay(pdMS_TO_TICKS(100));  // let the ack drain first
        esp_restart();
    }
}

extern "C" void app_main(void)
{
#if CONFIG_AS_BUILD_PROFILE_DRIVE
    ESP_LOGW(TAG, "Alpha Stick V2, DRIVE build profile");
#else
    ESP_LOGI(TAG, "Alpha Stick V2, gaming build profile");
#endif
#if CONFIG_AS_ROLE_DONGLE
    ESP_LOGW(TAG, "role: DONGLE (AS-Link bridge), not implemented yet");
#endif

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    as::config_init();
    as::usb_hid_init();
    as::usb_hid_set_cdc_line_handler(as_cdc_on_line);
    as::ble_hid_init();
    as::web_init();

    as::sensing_start(s_mailbox);
    as::router_start(s_mailbox);

    ESP_LOGI(TAG, "up: profile '%s', mode %d",
             as::config_profile().name, (int)as::config_profile().mode);
}
