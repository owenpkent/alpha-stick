// AS-Link wire format, v1. Single source of truth for the Alpha Stick to
// ATOS input link (docs/DESIGN_V2.md section 9). Pure C99 with no ESP-IDF
// dependencies so the ATOS gateway module can vendor this file unchanged.
//
// Semantics deliberately mirror ATOS atos_drive_proposal_t: normalized axes,
// TTL the receiver decays on, monotonic seq the receiver de-dupes on. The
// receiver treats the input as dead if health frames stop, regardless of
// input frames.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASLINK_VERSION 0x01

enum aslink_frame_type {
    ASLINK_TYPE_INPUT = 0x01,
    ASLINK_TYPE_HEALTH = 0x02,
};

enum aslink_flags {
    ASLINK_FLAG_CAL_VALID = 1u << 0,
    ASLINK_FLAG_DUAL_SENSOR_OK = 1u << 1,
    ASLINK_FLAG_TEMP_OK = 1u << 2,
    ASLINK_FLAG_TARE_STABLE = 1u << 3,
};

typedef struct __attribute__((packed)) {
    uint8_t ver;        // ASLINK_VERSION
    uint8_t type;       // ASLINK_TYPE_INPUT
    uint16_t seq;       // monotonic; receiver drops dupes/regressions
    int16_t x_q15;      // [-1, +1] in Q15, post-pipeline
    int16_t y_q15;
    int16_t z_q15;      // press axis [0, +1] in Q15
    uint16_t buttons;   // jacks, sip/puff thresholds, Z-click
    uint16_t ttl_ms;    // receiver decays output to zero when stale
    uint8_t flags;      // aslink_flags
    uint8_t reserved;
    uint32_t crc32;     // IEEE CRC32 over all preceding bytes
} aslink_input_v1_t;

_Static_assert(sizeof(aslink_input_v1_t) == 20, "aslink_input_v1_t must be 20 bytes");

typedef struct __attribute__((packed)) {
    uint8_t ver;
    uint8_t type;             // ASLINK_TYPE_HEALTH, sent at 1 Hz
    uint16_t seq;
    uint8_t fw_major;
    uint8_t fw_minor;
    int8_t temp_c;
    uint8_t sensor_disagree_pct;  // peak since last health frame, % of FS
    uint32_t uptime_s;
    uint16_t reserved;
    uint32_t crc32;
} aslink_health_v1_t;

_Static_assert(sizeof(aslink_health_v1_t) == 18, "aslink_health_v1_t must be 18 bytes");

// CRC32 (IEEE 802.3, reflected, init 0xFFFFFFFF, final xor 0xFFFFFFFF).
uint32_t aslink_crc32(const void *data, size_t len);

// Compute and store / check the trailing crc32 of a frame.
void aslink_input_finalize(aslink_input_v1_t *frame);
bool aslink_input_validate(const aslink_input_v1_t *frame);
void aslink_health_finalize(aslink_health_v1_t *frame);
bool aslink_health_validate(const aslink_health_v1_t *frame);

// COBS framing for the UART transport (ESP-NOW sends structs raw). Output
// buffers: encode needs len + 2 bytes (worst case + terminating 0x00, which
// encode appends); decode needs len bytes. Returns bytes written, 0 on error.
size_t aslink_cobs_encode(const uint8_t *in, size_t len, uint8_t *out, size_t out_cap);
size_t aslink_cobs_decode(const uint8_t *in, size_t len, uint8_t *out, size_t out_cap);

#ifdef __cplusplus
}
#endif
