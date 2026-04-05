#ifndef OSFX_FUSION_PACKET_H
#define OSFX_FUSION_PACKET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct osfx_packet_meta {
    uint8_t cmd;
    uint8_t route_count;
    uint32_t source_aid;
    uint8_t tid;
    uint64_t timestamp_raw;
    size_t body_offset;
    size_t body_len;
    int crc8_ok;
    int crc16_ok;
} osfx_packet_meta;

int osfx_packet_encode_full(
    uint8_t cmd,
    uint32_t source_aid,
    uint8_t tid,
    uint64_t timestamp_raw,
    const uint8_t* body,
    size_t body_len,
    uint8_t* out,
    size_t out_cap
);

int osfx_packet_decode_min(const uint8_t* packet, size_t packet_len, uint64_t out_fields[9]);
int osfx_packet_decode_meta(const uint8_t* packet, size_t packet_len, osfx_packet_meta* out_meta);

int osfx_packet_encode_ex(
    uint8_t cmd,
    uint32_t source_aid,
    uint8_t tid,
    uint64_t timestamp_raw,
    const uint8_t* body,
    size_t body_len,
    int secure,
    const uint8_t* key,
    size_t key_len,
    uint8_t* out,
    size_t out_cap
);

int osfx_packet_extract_body(
    const uint8_t* packet,
    size_t packet_len,
    const uint8_t* key,
    size_t key_len,
    uint8_t* out_body,
    size_t out_body_cap,
    size_t* out_body_len,
    osfx_packet_meta* out_meta
);

#ifdef __cplusplus
}
#endif

#endif

