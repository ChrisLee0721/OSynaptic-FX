#ifndef OSFX_PAYLOAD_CRYPTO_H
#define OSFX_PAYLOAD_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void osfx_xor_payload(
    const uint8_t* payload,
    size_t payload_len,
    const uint8_t* key,
    size_t key_len,
    uint32_t offset,
    uint8_t* out
);

#ifdef __cplusplus
}
#endif

#endif

