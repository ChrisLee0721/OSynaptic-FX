#include "../include/osfx_payload_crypto.h"

#include <string.h>

void osfx_xor_payload(
    const uint8_t* payload,
    size_t payload_len,
    const uint8_t* key,
    size_t key_len,
    uint32_t offset,
    uint8_t* out
) {
    size_t i;
    uint8_t off = (uint8_t)(offset & 31U);
    if (!out) {
        return;
    }
    if (!payload || payload_len == 0U) {
        return;
    }
    if (!key || key_len == 0U) {
        memcpy(out, payload, payload_len);
        return;
    }
    for (i = 0; i < payload_len; ++i) {
        out[i] = (uint8_t)(payload[i] ^ key[(i + off) % key_len] ^ off);
    }
}

