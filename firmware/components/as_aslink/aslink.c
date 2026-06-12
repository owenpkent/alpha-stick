#include "aslink_frame.h"

#include <string.h>

uint32_t aslink_crc32(const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc ^= p[i];
        for (int b = 0; b < 8; ++b) {
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1u)));
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

void aslink_input_finalize(aslink_input_v1_t *frame)
{
    frame->crc32 = aslink_crc32(frame, offsetof(aslink_input_v1_t, crc32));
}

bool aslink_input_validate(const aslink_input_v1_t *frame)
{
    if (frame->ver != ASLINK_VERSION || frame->type != ASLINK_TYPE_INPUT) {
        return false;
    }
    return frame->crc32 == aslink_crc32(frame, offsetof(aslink_input_v1_t, crc32));
}

void aslink_health_finalize(aslink_health_v1_t *frame)
{
    frame->crc32 = aslink_crc32(frame, offsetof(aslink_health_v1_t, crc32));
}

bool aslink_health_validate(const aslink_health_v1_t *frame)
{
    if (frame->ver != ASLINK_VERSION || frame->type != ASLINK_TYPE_HEALTH) {
        return false;
    }
    return frame->crc32 == aslink_crc32(frame, offsetof(aslink_health_v1_t, crc32));
}

size_t aslink_cobs_encode(const uint8_t *in, size_t len, uint8_t *out, size_t out_cap)
{
    if (len == 0 || out_cap < len + 2) {
        return 0;
    }
    size_t out_i = 1;   // out[0] is the first code byte
    size_t code_i = 0;
    uint8_t code = 1;
    for (size_t i = 0; i < len; ++i) {
        if (in[i] == 0) {
            out[code_i] = code;
            code_i = out_i++;
            code = 1;
        } else {
            out[out_i++] = in[i];
            if (++code == 0xFF) {
                out[code_i] = code;
                code_i = out_i++;
                code = 1;
            }
        }
    }
    out[code_i] = code;
    out[out_i++] = 0x00;  // frame delimiter
    return out_i;
}

size_t aslink_cobs_decode(const uint8_t *in, size_t len, uint8_t *out, size_t out_cap)
{
    // Accepts a frame with or without the trailing 0x00 delimiter.
    if (len > 0 && in[len - 1] == 0x00) {
        --len;
    }
    size_t out_i = 0;
    size_t i = 0;
    while (i < len) {
        const uint8_t code = in[i++];
        if (code == 0 || i + (size_t)(code - 1) > len) {
            return 0;  // malformed
        }
        for (uint8_t j = 1; j < code; ++j) {
            if (out_i >= out_cap) {
                return 0;
            }
            out[out_i++] = in[i++];
        }
        if (code != 0xFF && i < len) {
            if (out_i >= out_cap) {
                return 0;
            }
            out[out_i++] = 0x00;
        }
    }
    return out_i;
}
