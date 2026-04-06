#include "../include/osfx_handshake_cmd.h"

#include <string.h>

static void write_u16_be(uint8_t* out, uint16_t v) {
    out[0] = (uint8_t)((v >> 8) & 0xFFU);
    out[1] = (uint8_t)(v & 0xFFU);
}

static void write_u32_be(uint8_t* out, uint32_t v) {
    out[0] = (uint8_t)((v >> 24) & 0xFFU);
    out[1] = (uint8_t)((v >> 16) & 0xFFU);
    out[2] = (uint8_t)((v >> 8) & 0xFFU);
    out[3] = (uint8_t)(v & 0xFFU);
}

int osfx_cmd_normalize_data(uint8_t cmd) {
    if (cmd == OSFX_CMD_DATA_FULL_SEC) {
        return OSFX_CMD_DATA_FULL;
    }
    if (cmd == OSFX_CMD_DATA_DIFF_SEC) {
        return OSFX_CMD_DATA_DIFF;
    }
    if (cmd == OSFX_CMD_DATA_HEART_SEC) {
        return OSFX_CMD_DATA_HEART;
    }
    return (int)cmd;
}

int osfx_cmd_secure_variant(uint8_t cmd) {
    if (cmd == OSFX_CMD_DATA_FULL) {
        return OSFX_CMD_DATA_FULL_SEC;
    }
    if (cmd == OSFX_CMD_DATA_DIFF) {
        return OSFX_CMD_DATA_DIFF_SEC;
    }
    if (cmd == OSFX_CMD_DATA_HEART) {
        return OSFX_CMD_DATA_HEART_SEC;
    }
    return (int)cmd;
}

int osfx_build_ack(uint16_t seq, uint8_t* out, size_t out_cap) {
    if (!out || out_cap < 3U) {
        return 0;
    }
    out[0] = OSFX_CMD_HANDSHAKE_ACK;
    write_u16_be(out + 1, seq);
    return 3;
}

int osfx_build_nack(uint16_t seq, const char* reason, uint8_t* out, size_t out_cap) {
    size_t reason_len = 0;
    if (!out || out_cap < 3U) {
        return 0;
    }
    if (reason) {
        reason_len = strlen(reason);
    }
    if (3U + reason_len > out_cap) {
        return 0;
    }
    out[0] = OSFX_CMD_HANDSHAKE_NACK;
    write_u16_be(out + 1, seq);
    if (reason_len > 0U) {
        memcpy(out + 3, reason, reason_len);
    }
    return (int)(3U + reason_len);
}

int osfx_build_id_assign(uint16_t seq, uint32_t assigned_id, uint8_t* out, size_t out_cap) {
    if (!out || out_cap < 7U) {
        return 0;
    }
    out[0] = OSFX_CMD_ID_ASSIGN;
    write_u16_be(out + 1, seq);
    write_u32_be(out + 3, assigned_id);
    return 7;
}

int osfx_build_time_request(uint16_t seq, uint8_t* out, size_t out_cap) {
    if (!out || out_cap < 3U) {
        return 0;
    }
    out[0] = OSFX_CMD_TIME_REQUEST;
    write_u16_be(out + 1, seq);
    return 3;
}

int osfx_build_time_response(uint16_t seq, uint64_t unix_ts, uint8_t* out, size_t out_cap) {
    if (!out || out_cap < 11U) {
        return 0;
    }
    out[0] = OSFX_CMD_TIME_RESPONSE;
    write_u16_be(out + 1, seq);
    out[3] = (uint8_t)((unix_ts >> 56) & 0xFFU);
    out[4] = (uint8_t)((unix_ts >> 48) & 0xFFU);
    out[5] = (uint8_t)((unix_ts >> 40) & 0xFFU);
    out[6] = (uint8_t)((unix_ts >> 32) & 0xFFU);
    out[7] = (uint8_t)((unix_ts >> 24) & 0xFFU);
    out[8] = (uint8_t)((unix_ts >> 16) & 0xFFU);
    out[9] = (uint8_t)((unix_ts >> 8) & 0xFFU);
    out[10] = (uint8_t)(unix_ts & 0xFFU);
    return 11;
}

int osfx_build_secure_dict_ready(uint32_t assigned_id, uint64_t timestamp_raw, uint8_t* out, size_t out_cap) {
    if (!out || out_cap < 13U) {
        return 0;
    }
    out[0] = OSFX_CMD_SECURE_DICT_READY;
    write_u32_be(out + 1, assigned_id);
    out[5] = (uint8_t)((timestamp_raw >> 56) & 0xFFU);
    out[6] = (uint8_t)((timestamp_raw >> 48) & 0xFFU);
    out[7] = (uint8_t)((timestamp_raw >> 40) & 0xFFU);
    out[8] = (uint8_t)((timestamp_raw >> 32) & 0xFFU);
    out[9] = (uint8_t)((timestamp_raw >> 24) & 0xFFU);
    out[10] = (uint8_t)((timestamp_raw >> 16) & 0xFFU);
    out[11] = (uint8_t)((timestamp_raw >> 8) & 0xFFU);
    out[12] = (uint8_t)(timestamp_raw & 0xFFU);
    return 13;
}

int osfx_build_secure_channel_ack(uint32_t assigned_id, uint8_t* out, size_t out_cap) {
    if (!out || out_cap < 5U) {
        return 0;
    }
    out[0] = OSFX_CMD_SECURE_CHANNEL_ACK;
    write_u32_be(out + 1, assigned_id);
    return 5;
}

int osfx_parse_id_assign(
    const uint8_t* packet,
    size_t packet_len,
    uint16_t* out_seq,
    uint32_t* out_assigned_id,
    uint64_t* out_server_time
) {
    if (!packet || packet_len < 7U || packet[0] != OSFX_CMD_ID_ASSIGN) {
        return 0;
    }
    if (out_seq) {
        *out_seq = (uint16_t)(((uint16_t)packet[1] << 8) | packet[2]);
    }
    if (out_assigned_id) {
        *out_assigned_id = ((uint32_t)packet[3] << 24) |
                           ((uint32_t)packet[4] << 16) |
                           ((uint32_t)packet[5] << 8)  |
                            (uint32_t)packet[6];
    }
    if (out_server_time) {
        if (packet_len >= 15U) {
            *out_server_time = ((uint64_t)packet[7]  << 56) |
                               ((uint64_t)packet[8]  << 48) |
                               ((uint64_t)packet[9]  << 40) |
                               ((uint64_t)packet[10] << 32) |
                               ((uint64_t)packet[11] << 24) |
                               ((uint64_t)packet[12] << 16) |
                               ((uint64_t)packet[13] << 8)  |
                                (uint64_t)packet[14];
        } else {
            *out_server_time = 0U;
        }
    }
    return 1;
}

int osfx_parse_time_response(
    const uint8_t* packet,
    size_t packet_len,
    uint16_t* out_seq,
    uint64_t* out_unix_ts
) {
    if (!packet || packet_len < 11U || packet[0] != OSFX_CMD_TIME_RESPONSE) {
        return 0;
    }
    if (out_seq) {
        *out_seq = (uint16_t)(((uint16_t)packet[1] << 8) | packet[2]);
    }
    if (out_unix_ts) {
        *out_unix_ts = ((uint64_t)packet[3]  << 56) |
                       ((uint64_t)packet[4]  << 48) |
                       ((uint64_t)packet[5]  << 40) |
                       ((uint64_t)packet[6]  << 32) |
                       ((uint64_t)packet[7]  << 24) |
                       ((uint64_t)packet[8]  << 16) |
                       ((uint64_t)packet[9]  << 8)  |
                        (uint64_t)packet[10];
    }
    return 1;
}

int osfx_build_id_pool_request(
    uint16_t seq,
    uint16_t count,
    uint8_t* out,
    size_t out_cap
) {
    if (!out || out_cap < 5U) {
        return 0;
    }
    out[0] = OSFX_CMD_ID_POOL_REQ;
    write_u16_be(out + 1, seq);
    write_u16_be(out + 3, count);
    return 5;
}

int osfx_parse_id_pool_response(
    const uint8_t* packet,
    size_t packet_len,
    uint16_t* out_seq,
    uint32_t* out_pool,
    size_t out_pool_cap,
    size_t* out_count
) {
    uint16_t declared_count;
    size_t i;
    size_t actual;

    if (!packet || packet_len < 5U || packet[0] != OSFX_CMD_ID_POOL_RES) {
        return 0;
    }
    if (out_seq) {
        *out_seq = (uint16_t)(((uint16_t)packet[1] << 8) | packet[2]);
    }
    declared_count = (uint16_t)(((uint16_t)packet[3] << 8) | packet[4]);
    /* Verify packet is long enough to hold all declared AIDs */
    if (packet_len < 5U + (size_t)declared_count * 4U) {
        return 0;
    }
    actual = (declared_count < (uint16_t)out_pool_cap) ? declared_count : (uint16_t)out_pool_cap;
    for (i = 0; i < actual; ++i) {
        size_t off = 5U + i * 4U;
        out_pool[i] = ((uint32_t)packet[off]     << 24) |
                      ((uint32_t)packet[off + 1] << 16) |
                      ((uint32_t)packet[off + 2] << 8)  |
                       (uint32_t)packet[off + 3];
    }
    if (out_count) {
        *out_count = actual;
    }
    return 1;
}

