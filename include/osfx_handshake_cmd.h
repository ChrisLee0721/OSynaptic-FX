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
    OSFX_CMD_ID_POOL_REQ = 3,   /* Batch-ID pool request (gateway/proxy role) */
    OSFX_CMD_ID_POOL_RES = 4,   /* Batch-ID pool response (gateway/proxy role) */
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

/*
 * osfx_parse_id_assign
 * --------------------
 * Parse an inbound ID_ASSIGN packet received from the server (client-side helper).
 *
 * Wire format (backward-compatible, see docs/18-data-format-specification.md A.4):
 *   Minimum (7 bytes): [cmd:1][seq:2][assigned_id:4]
 *   With server_time (15 bytes): [cmd:1][seq:2][assigned_id:4][server_time:8]
 *
 * out_server_time is set to 0 if the server_time extension is absent.
 * Returns 1 on success, 0 if packet is too short or cmd is wrong.
 */
int osfx_parse_id_assign(const uint8_t* packet, size_t packet_len,
                         uint16_t* out_seq, uint32_t* out_assigned_id,
                         uint64_t* out_server_time);

/*
 * osfx_parse_time_response
 * ------------------------
 * Parse an inbound TIME_RESPONSE packet received from the server.
 *
 * Wire format: [cmd:1][seq:2][unix_ts:8]  = 11 bytes
 *
 * Returns 1 on success, 0 if packet is too short or cmd is wrong.
 */
int osfx_parse_time_response(const uint8_t* packet, size_t packet_len,
                             uint16_t* out_seq, uint64_t* out_unix_ts);

/*
 * osfx_build_id_pool_request
 * --------------------------
 * Build an ID_POOL_REQ(3) packet: request `count` AIDs from the upstream server
 * (gateway/proxy role).
 *
 * Wire format: [cmd:1][seq:2][count:2]  = 5 bytes (no JSON meta in C build)
 *
 * Returns bytes written (5), or 0 on error.
 */
int osfx_build_id_pool_request(uint16_t seq, uint16_t count,
                               uint8_t* out, size_t out_cap);

/*
 * osfx_parse_id_pool_response
 * ---------------------------
 * Parse an inbound ID_POOL_RES(4) packet and extract up to `out_pool_cap` AIDs
 * into `out_pool[]`.  Write the actual count to `*out_count`.
 *
 * Wire format: [cmd:1][seq:2][count:2][aid0:4][aid1:4]...  (5 + count*4 bytes)
 *
 * Call this after receiving ID_POOL_RES then seed each AID into a local
 * osfx_id_allocator via osfx_id_allocate() or store them in a gateway pool.
 *
 * Returns 1 on success, 0 if packet is malformed or cmd is wrong.
 */
int osfx_parse_id_pool_response(const uint8_t* packet, size_t packet_len,
                                uint16_t* out_seq,
                                uint32_t* out_pool, size_t out_pool_cap,
                                size_t* out_count);

#ifdef __cplusplus
}
#endif

#endif

