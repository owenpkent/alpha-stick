#include "as_sensing/tmag5273.h"

#include "esp_log.h"

namespace as {

namespace {

const char *TAG = "tmag5273";

// VERIFY (Phase 0): register addresses from datasheet memory.
constexpr uint8_t REG_DEVICE_CONFIG_1 = 0x00;
constexpr uint8_t REG_DEVICE_CONFIG_2 = 0x01;
constexpr uint8_t REG_SENSOR_CONFIG_1 = 0x02;
constexpr uint8_t REG_SENSOR_CONFIG_2 = 0x03;
constexpr uint8_t REG_MANUFACTURER_ID_LSB = 0x0E;  // expect 0x49
constexpr uint8_t REG_MANUFACTURER_ID_MSB = 0x0F;  // expect 0x54 ("TI")
constexpr uint8_t REG_X_MSB_RESULT = 0x12;         // X,Y,Z MSB/LSB pairs follow

constexpr uint16_t kManufacturerId = 0x5449;

}  // namespace

constexpr uint8_t Tmag5273::kCandidateAddrs[];

esp_err_t Tmag5273::probe(i2c_master_bus_handle_t bus)
{
    for (uint8_t addr : kCandidateAddrs) {
        if (i2c_master_probe(bus, addr, 20) != ESP_OK) {
            continue;
        }
        i2c_device_config_t cfg = {};
        cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        cfg.device_address = addr;
        cfg.scl_speed_hz = 400000;
        if (i2c_master_bus_add_device(bus, &cfg, &dev_) != ESP_OK) {
            dev_ = nullptr;
            continue;
        }
        uint8_t id[2] = {0, 0};
        if (rd(REG_MANUFACTURER_ID_LSB, id, 2) == ESP_OK &&
            (uint16_t)(id[0] | (id[1] << 8)) == kManufacturerId) {
            addr_ = addr;
            ESP_LOGI(TAG, "found at 0x%02X", addr);
            return ESP_OK;
        }
        i2c_master_bus_rm_device(dev_);
        dev_ = nullptr;
    }
    return ESP_ERR_NOT_FOUND;
}

esp_err_t Tmag5273::start_continuous()
{
    if (!dev_) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t err = ESP_OK;
    // VERIFY: CONV_AVG[4:2] = 0b011 -> 8x averaging.
    err |= wr(REG_DEVICE_CONFIG_1, 0x03 << 2);
    // VERIFY: MAG_CH_EN[7:4] = 0b0111 -> X, Y, Z enabled.
    err |= wr(REG_SENSOR_CONFIG_1, 0x07 << 4);
    // VERIFY: range bits 0 -> +/-40 mT on A1.
    err |= wr(REG_SENSOR_CONFIG_2, 0x00);
    // VERIFY: OPERATING_MODE[1:0] = 0b10 -> continuous conversion.
    err |= wr(REG_DEVICE_CONFIG_2, 0x02);
    return err == ESP_OK ? ESP_OK : ESP_FAIL;
}

esp_err_t Tmag5273::read_mT(float &bx, float &by, float &bz)
{
    if (!dev_) {
        return ESP_ERR_INVALID_STATE;
    }
    uint8_t raw[6];
    esp_err_t err = rd(REG_X_MSB_RESULT, raw, sizeof(raw));
    if (err != ESP_OK) {
        return err;
    }
    const int16_t x = (int16_t)((raw[0] << 8) | raw[1]);
    const int16_t y = (int16_t)((raw[2] << 8) | raw[3]);
    const int16_t z = (int16_t)((raw[4] << 8) | raw[5]);
    // VERIFY: full-scale code-to-mT scaling.
    const float k = range_mT_ / 32768.0f;
    bx = x * k;
    by = y * k;
    bz = z * k;
    return ESP_OK;
}

esp_err_t Tmag5273::rd(uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(dev_, &reg, 1, buf, len, 20);
}

esp_err_t Tmag5273::wr(uint8_t reg, uint8_t val)
{
    const uint8_t frame[2] = {reg, val};
    return i2c_master_transmit(dev_, frame, sizeof(frame), 20);
}

}  // namespace as
