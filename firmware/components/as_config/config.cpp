#include "as_config/config.h"

#include <cstring>

#include "esp_log.h"
#include "nvs.h"

namespace as {

namespace {

const char *TAG = "config";
const char *kNamespace = "alphastick";
const char *kKey = "profile0";

// Bump when the Profile struct layout changes; mismatched blobs fall back to
// defaults instead of misreading.
constexpr uint8_t kBlobVersion = 1;

struct Blob {
    uint8_t version;
    Profile profile;
};

Profile s_profile{};

}  // namespace

void config_init()
{
    nvs_handle_t h;
    if (nvs_open(kNamespace, NVS_READONLY, &h) == ESP_OK) {
        Blob blob{};
        size_t len = sizeof(blob);
        if (nvs_get_blob(h, kKey, &blob, &len) == ESP_OK &&
            len == sizeof(blob) && blob.version == kBlobVersion) {
            s_profile = blob.profile;
            ESP_LOGI(TAG, "loaded profile '%s'", s_profile.name);
        } else {
            ESP_LOGI(TAG, "no stored profile, using defaults");
        }
        nvs_close(h);
    } else {
        ESP_LOGI(TAG, "NVS namespace empty, using defaults");
    }
}

const Profile &config_profile()
{
    return s_profile;
}

void config_set_profile(const Profile &p)
{
    s_profile = p;
    nvs_handle_t h;
    if (nvs_open(kNamespace, NVS_READWRITE, &h) != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed, profile not persisted");
        return;
    }
    Blob blob{};
    blob.version = kBlobVersion;
    blob.profile = p;
    if (nvs_set_blob(h, kKey, &blob, sizeof(blob)) == ESP_OK) {
        nvs_commit(h);
        ESP_LOGI(TAG, "persisted profile '%s'", p.name);
    } else {
        ESP_LOGE(TAG, "NVS write failed");
    }
    nvs_close(h);
}

}  // namespace as
