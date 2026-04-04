#include "../include/osfx_handshake_dispatch.h"

#include <string.h>

#include "../include/osfx_fusion_packet.h"
#include "../include/osfx_handshake_cmd.h"

static int write_response(osfx_hs_result* r, const uint8_t* data, size_t n);

static void set_reject(osfx_hs_result* r, osfx_hs_reject reject) {
    if (r) {
        r->ok = 0;
        r->reject = reject;
    }
}

static int maybe_write_nack_seq(osfx_hs_result* r, uint16_t seq, const char* reason) {
    uint8_t resp[OSFX_HS_RESP_MAX];
    int n = osfx_build_nack(seq, reason, resp, sizeof(resp));
    if (n > 0) {
        return write_response(r, resp, (size_t)n);
    }
    return 0;
}

static void clear_result(osfx_hs_result* r) {
    if (r) {
        memset(r, 0, sizeof(*r));
    }
}

static int write_response(osfx_hs_result* r, const uint8_t* data, size_t n) {
    if (!r || !data || n > OSFX_HS_RESP_MAX) {
        return 0;
    }
    memcpy(r->response, data, n);
    r->response_len = n;
    r->has_response = 1;
    return 1;
}

int osfx_hs_classify_dispatch(
    const osfx_hs_dispatch_ctx* ctx,
    const uint8_t* packet,
    size_t packet_len,
    osfx_hs_result* out_result
) {
    uint8_t cmd;
    uint8_t base_cmd;
    osfx_packet_meta meta;
    uint64_t now_ts = (ctx ? ctx->now_ts : 0U);

    clear_result(out_result);
    if (!packet || packet_len < 1U || !out_result) {
        if (out_result) {
            out_result->kind = OSFX_HS_KIND_ERROR;
        }
        return 0;
    }

    cmd = packet[0];
    base_cmd = (uint8_t)osfx_cmd_normalize_data(cmd);
    out_result->cmd = cmd;
    out_result->base_cmd = base_cmd;
    out_result->reject = OSFX_HS_REJECT_NONE;

    if (base_cmd == OSFX_CMD_DATA_FULL || base_cmd == OSFX_CMD_DATA_DIFF || base_cmd == OSFX_CMD_DATA_HEART) {
        out_result->kind = OSFX_HS_KIND_DATA;
        if (!osfx_packet_decode_meta(packet, packet_len, &meta)) {
            set_reject(out_result, OSFX_HS_REJECT_MALFORMED);
            return 0;
        }
        out_result->source_aid = meta.source_aid;
        if (!meta.crc16_ok) {
            set_reject(out_result, OSFX_HS_REJECT_CRC);
            return 0;
        }
        out_result->ok = 1;
        if (!ctx || !ctx->secure_store) {
            return 1;
        }

        {
            int ts_rc = osfx_secure_check_and_update_timestamp(ctx->secure_store, meta.source_aid, meta.timestamp_raw, now_ts);
            if (ts_rc == OSFX_TS_REPLAY) {
                set_reject(out_result, OSFX_HS_REJECT_REPLAY);
                return 0;
            }
            if (ts_rc == OSFX_TS_OUT_OF_ORDER) {
                set_reject(out_result, OSFX_HS_REJECT_OUT_OF_ORDER);
                return 0;
            }
        }

        if (cmd == base_cmd) {
            uint8_t resp[16];
            int n;
            if (!osfx_secure_should_encrypt(ctx->secure_store, meta.source_aid)) {
                osfx_secure_confirm_dict(ctx->secure_store, meta.source_aid, meta.timestamp_raw, now_ts);
                n = osfx_build_secure_dict_ready(meta.source_aid, meta.timestamp_raw, resp, sizeof(resp));
                if (n > 0) {
                    write_response(out_result, resp, (size_t)n);
                }
            }
        } else {
            uint8_t resp[8];
            int n;
            if (!osfx_secure_should_encrypt(ctx->secure_store, meta.source_aid)) {
                set_reject(out_result, OSFX_HS_REJECT_NO_SESSION);
                return 0;
            }
            if (!osfx_secure_is_channel_confirmed(ctx->secure_store, meta.source_aid)) {
                osfx_secure_mark_channel(ctx->secure_store, meta.source_aid, now_ts);
                n = osfx_build_secure_channel_ack(meta.source_aid, resp, sizeof(resp));
                if (n > 0) {
                    write_response(out_result, resp, (size_t)n);
                }
            }
        }
        return 1;
    }

    out_result->kind = OSFX_HS_KIND_CTRL;
    out_result->ok = 1;

    if (cmd == OSFX_CMD_PING) {
        if (packet_len != 3U) {
            set_reject(out_result, OSFX_HS_REJECT_MALFORMED);
            return 0;
        }
        uint16_t seq = (uint16_t)(((uint16_t)packet[1] << 8) | packet[2]);
        uint8_t resp[3];
        resp[0] = OSFX_CMD_PONG;
        resp[1] = (uint8_t)((seq >> 8) & 0xFFU);
        resp[2] = (uint8_t)(seq & 0xFFU);
        write_response(out_result, resp, 3U);
        return 1;
    }

    if (cmd == OSFX_CMD_TIME_REQUEST) {
        if (packet_len != 3U) {
            set_reject(out_result, OSFX_HS_REJECT_MALFORMED);
            return 0;
        }
        uint16_t seq = (uint16_t)(((uint16_t)packet[1] << 8) | packet[2]);
        uint8_t resp[16];
        int n = osfx_build_time_response(seq, now_ts, resp, sizeof(resp));
        if (n > 0) {
            write_response(out_result, resp, (size_t)n);
        }
        return 1;
    }

    if (cmd == OSFX_CMD_ID_REQUEST) {
        if (packet_len < 3U) {
            set_reject(out_result, OSFX_HS_REJECT_MALFORMED);
            maybe_write_nack_seq(out_result, 0U, "MALFORMED_ID_REQUEST");
            return 0;
        }
        if (!(ctx && ctx->id_allocator)) {
            set_reject(out_result, OSFX_HS_REJECT_UNSUPPORTED);
            maybe_write_nack_seq(out_result, (uint16_t)(((uint16_t)packet[1] << 8) | packet[2]), "NO_ALLOCATOR");
            return 0;
        }
        uint16_t seq = (uint16_t)(((uint16_t)packet[1] << 8) | packet[2]);
        uint32_t aid = 0;
        uint8_t resp[16];
        int n;
        if (osfx_id_allocate(ctx->id_allocator, now_ts, &aid)) {
            n = osfx_build_id_assign(seq, aid, resp, sizeof(resp));
        } else {
            n = osfx_build_nack(seq, "ID_POOL_EXHAUSTED", resp, sizeof(resp));
        }
        if (n > 0) {
            write_response(out_result, resp, (size_t)n);
        }
        return 1;
    }

    if (cmd == OSFX_CMD_SECURE_DICT_READY) {
        if (packet_len != 13U || !(ctx && ctx->secure_store)) {
            set_reject(out_result, OSFX_HS_REJECT_MALFORMED);
            return 0;
        }
        uint32_t aid = ((uint32_t)packet[1] << 24) | ((uint32_t)packet[2] << 16) | ((uint32_t)packet[3] << 8) | (uint32_t)packet[4];
        uint64_t ts = ((uint64_t)packet[5] << 56) | ((uint64_t)packet[6] << 48) | ((uint64_t)packet[7] << 40) |
                      ((uint64_t)packet[8] << 32) | ((uint64_t)packet[9] << 24) | ((uint64_t)packet[10] << 16) |
                      ((uint64_t)packet[11] << 8) | (uint64_t)packet[12];
        osfx_secure_confirm_dict(ctx->secure_store, aid, ts, now_ts);
        return 1;
    }

    if (cmd == OSFX_CMD_SECURE_CHANNEL_ACK) {
        if (packet_len != 5U || !(ctx && ctx->secure_store)) {
            set_reject(out_result, OSFX_HS_REJECT_MALFORMED);
            return 0;
        }
        uint32_t aid = ((uint32_t)packet[1] << 24) | ((uint32_t)packet[2] << 16) | ((uint32_t)packet[3] << 8) | (uint32_t)packet[4];
        osfx_secure_mark_channel(ctx->secure_store, aid, now_ts);
        return 1;
    }

    if (cmd == OSFX_CMD_HANDSHAKE_ACK || cmd == OSFX_CMD_HANDSHAKE_NACK || cmd == OSFX_CMD_ID_ASSIGN || cmd == OSFX_CMD_TIME_RESPONSE || cmd == OSFX_CMD_PONG) {
        return 1;
    }

    out_result->kind = OSFX_HS_KIND_UNKNOWN;
    set_reject(out_result, OSFX_HS_REJECT_UNSUPPORTED);
    return 0;
}

