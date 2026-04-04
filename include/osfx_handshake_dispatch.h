#ifndef OSFX_HANDSHAKE_DISPATCH_H
#define OSFX_HANDSHAKE_DISPATCH_H

#include <stddef.h>
#include <stdint.h>

#include "osfx_id_allocator.h"
#include "osfx_secure_session.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OSFX_HS_RESP_MAX 128

typedef enum osfx_hs_kind {
    OSFX_HS_KIND_ERROR = 0,
    OSFX_HS_KIND_DATA = 1,
    OSFX_HS_KIND_CTRL = 2,
    OSFX_HS_KIND_UNKNOWN = 3
} osfx_hs_kind;

typedef enum osfx_hs_reject {
    OSFX_HS_REJECT_NONE = 0,
    OSFX_HS_REJECT_MALFORMED = 1,
    OSFX_HS_REJECT_CRC = 2,
    OSFX_HS_REJECT_REPLAY = 3,
    OSFX_HS_REJECT_OUT_OF_ORDER = 4,
    OSFX_HS_REJECT_NO_SESSION = 5,
    OSFX_HS_REJECT_UNSUPPORTED = 6
} osfx_hs_reject;

typedef struct osfx_hs_dispatch_ctx {
    osfx_secure_session_store* secure_store;
    osfx_id_allocator* id_allocator;
    uint64_t now_ts;
} osfx_hs_dispatch_ctx;

typedef struct osfx_hs_result {
    osfx_hs_kind kind;
    uint8_t cmd;
    uint8_t base_cmd;
    uint32_t source_aid;
    int ok;
    osfx_hs_reject reject;
    int has_response;
    uint8_t response[OSFX_HS_RESP_MAX];
    size_t response_len;
} osfx_hs_result;

int osfx_hs_classify_dispatch(
    const osfx_hs_dispatch_ctx* ctx,
    const uint8_t* packet,
    size_t packet_len,
    osfx_hs_result* out_result
);

#ifdef __cplusplus
}
#endif

#endif

