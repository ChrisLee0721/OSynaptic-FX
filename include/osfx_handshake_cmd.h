#ifndef OSFX_HANDSHAKE_CMD_H
#define OSFX_HANDSHAKE_CMD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    OSFX_CMD_DATA_FULL = 63,
    OSFX_CMD_DATA_FULL_SEC = 64,
    OSFX_CMD_DATA_DIFF = 170,
    OSFX_CMD_DATA_DIFF_SEC = 171,
    OSFX_CMD_DATA_HEART = 127,
    OSFX_CMD_DATA_HEART_SEC = 128,
    OSFX_CMD_ID_REQUEST = 1,
    OSFX_CMD_ID_ASSIGN = 2,
    OSFX_CMD_HANDSHAKE_ACK = 5,
    OSFX_CMD_HANDSHAKE_NACK = 6,
    OSFX_CMD_PING = 9,
    OSFX_CMD_PONG = 10,
    OSFX_CMD_TIME_REQUEST = 11,
    OSFX_CMD_TIME_RESPONSE = 12,
    OSFX_CMD_SECURE_DICT_READY = 13,
    OSFX_CMD_SECURE_CHANNEL_ACK = 14
};

int osfx_cmd_normalize_data(uint8_t cmd);
int osfx_cmd_secure_variant(uint8_t cmd);

int osfx_build_ack(uint16_t seq, uint8_t* out, size_t out_cap);
int osfx_build_nack(uint16_t seq, const char* reason, uint8_t* out, size_t out_cap);
int osfx_build_id_assign(uint16_t seq, uint32_t assigned_id, uint8_t* out, size_t out_cap);
int osfx_build_time_request(uint16_t seq, uint8_t* out, size_t out_cap);
int osfx_build_time_response(uint16_t seq, uint64_t unix_ts, uint8_t* out, size_t out_cap);
int osfx_build_secure_dict_ready(uint32_t assigned_id, uint64_t timestamp_raw, uint8_t* out, size_t out_cap);
int osfx_build_secure_channel_ack(uint32_t assigned_id, uint8_t* out, size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif

