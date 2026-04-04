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

