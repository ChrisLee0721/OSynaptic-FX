#ifndef OSFX_FUSION_STATE_H
#define OSFX_FUSION_STATE_H

#include <stddef.h>
#include <stdint.h>

#include "osfx_fusion_packet.h"
#include "osfx_handshake_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OSFX_FUSION_MAX_ENTRIES 64
#define OSFX_FUSION_MAX_PREFIX 96
#define OSFX_FUSION_MAX_VALUE 64

typedef struct osfx_fusion_entry {
    uint32_t source_aid;
    uint8_t tid;
    char prefix[OSFX_FUSION_MAX_PREFIX];
    size_t prefix_len;
    char last_value[OSFX_FUSION_MAX_VALUE];
    size_t last_value_len;
    int used;
} osfx_fusion_entry;

typedef struct osfx_fusion_state {
    osfx_fusion_entry entries[OSFX_FUSION_MAX_ENTRIES];
} osfx_fusion_state;

void osfx_fusion_state_init(osfx_fusion_state* st);
void osfx_fusion_state_reset(osfx_fusion_state* st);

int osfx_fusion_encode(
    osfx_fusion_state* st,
    uint32_t source_aid,
    uint8_t tid,
    uint64_t timestamp_raw,
    const uint8_t* full_body,
    size_t full_body_len,
    uint8_t* out_packet,
    size_t out_packet_cap,
    int* out_packet_len,
    uint8_t* out_cmd
);

int osfx_fusion_decode_apply(
    osfx_fusion_state* st,
    const uint8_t* packet,
    size_t packet_len,
    uint8_t* out_body,
    size_t out_body_cap,
    size_t* out_body_len,
    osfx_packet_meta* out_meta
);

#ifdef __cplusplus
}
#endif

#endif

