#pragma once

#include <cstdint>

#include "driver/i2c_master.h"
#include "esp_err.h"

namespace as {

// Minimal TMAG5273 driver: probe, continuous-conversion config, XYZ read.
//
// VERIFY (Phase 0): the register map and config field values below were
// transcribed from datasheet memory and must be checked against the current
// TI datasheet before any bench numbers are trusted.
class Tmag5273 {
public:
    // Factory default 7-bit addresses differ per orderable variant; probe
    // walks this list and binds to the first part that answers with TI's
    // manufacturer ID.
    static constexpr uint8_t kCandidateAddrs[] = {0x35, 0x22, 0x78, 0x44};

    esp_err_t probe(i2c_master_bus_handle_t bus);
    esp_err_t start_continuous();  // XYZ enabled, 8x averaging
    esp_err_t read_mT(float &bx, float &by, float &bz);

    bool found() const { return dev_ != nullptr; }
    uint8_t address() const { return addr_; }

private:
    esp_err_t rd(uint8_t reg, uint8_t *buf, size_t len);
    esp_err_t wr(uint8_t reg, uint8_t val);

    i2c_master_dev_handle_t dev_ = nullptr;
    uint8_t addr_ = 0;
    float range_mT_ = 40.0f;  // A1 variant low range
};

}  // namespace as
