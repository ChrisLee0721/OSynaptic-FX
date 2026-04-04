#include "../include/osfx_fusion_packet.h"
#include "../include/osfx_handshake_cmd.h"
#include "../include/osfx_payload_crypto.h"
#include "../include/osfx_solidity.h"

#include <string.h>

static void write_u32_be(uint8_t* out, uint32_t v) {
    out[0] = (uint8_t)((v >> 24) & 0xFFU);
    out[1] = (uint8_t)((v >> 16) & 0xFFU);
    out[2] = (uint8_t)((v >> 8) & 0xFFU);
    out[3] = (uint8_t)(v & 0xFFU);
}

static uint32_t read_u32_be(const uint8_t* in) {
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) | ((uint32_t)in[2] << 8) | (uint32_t)in[3];
}

int osfx_packet_encode_full(
    uint8_t cmd,
    uint32_t source_aid,
    uint8_t tid,
    uint64_t timestamp_raw,
    const uint8_t* body,
    size_t body_len,
    uint8_t* out,
    size_t out_cap
) {
    size_t off = 0;
    uint8_t crc8v;
    uint16_t crc16v;
    size_t frame_len = 13U + body_len + 3U;

    if (!out || out_cap < frame_len) {
        return 0;
    }

    out[off++] = cmd;
    out[off++] = 1;
    write_u32_be(out + off, source_aid);
    off += 4;
    out[off++] = tid;
    out[off++] = (uint8_t)((timestamp_raw >> 40) & 0xFFU);
    out[off++] = (uint8_t)((timestamp_raw >> 32) & 0xFFU);
    out[off++] = (uint8_t)((timestamp_raw >> 24) & 0xFFU);
    out[off++] = (uint8_t)((timestamp_raw >> 16) & 0xFFU);
    out[off++] = (uint8_t)((timestamp_raw >> 8) & 0xFFU);
    out[off++] = (uint8_t)(timestamp_raw & 0xFFU);

    if (body_len > 0 && body) {
        size_t i;
        for (i = 0; i < body_len; ++i) {
            out[off + i] = body[i];
        }
    }
    off += body_len;

    crc8v = osfx_crc8(body, body_len, 0x07U, 0x00U);
    out[off++] = crc8v;
    crc16v = osfx_crc16_ccitt(out, off, 0x1021U, 0xFFFFU);
    out[off++] = (uint8_t)((crc16v >> 8) & 0xFFU);
    out[off++] = (uint8_t)(crc16v & 0xFFU);

    return (int)off;
}

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
) {
    size_t off = 0;
    uint8_t crc8v;
    uint16_t crc16v;
    size_t frame_len = 13U + body_len + 3U;
    uint8_t wire_cmd = cmd;
    if (!out || out_cap < frame_len) {
        return 0;
    }
    if (secure) {
        wire_cmd = (uint8_t)osfx_cmd_secure_variant(cmd);
    }

    out[off++] = wire_cmd;
    out[off++] = 1;
    write_u32_be(out + off, source_aid);
    off += 4;
    out[off++] = tid;
    out[off++] = (uint8_t)((timestamp_raw >> 40) & 0xFFU);
    out[off++] = (uint8_t)((timestamp_raw >> 32) & 0xFFU);
    out[off++] = (uint8_t)((timestamp_raw >> 24) & 0xFFU);
    out[off++] = (uint8_t)((timestamp_raw >> 16) & 0xFFU);
    out[off++] = (uint8_t)((timestamp_raw >> 8) & 0xFFU);
    out[off++] = (uint8_t)(timestamp_raw & 0xFFU);

    crc8v = osfx_crc8(body, body_len, 0x07U, 0x00U);
    if (body_len > 0 && body) {
        if (secure) {
            osfx_xor_payload(body, body_len, key, key_len, (uint32_t)(crc8v & 31U), out + off);
        } else {
            size_t i;
            for (i = 0; i < body_len; ++i) {
                out[off + i] = body[i];
            }
        }
    }
    off += body_len;

    out[off++] = crc8v;
    crc16v = osfx_crc16_ccitt(out, off, 0x1021U, 0xFFFFU);
    out[off++] = (uint8_t)((crc16v >> 8) & 0xFFU);
    out[off++] = (uint8_t)(crc16v & 0xFFU);
    return (int)off;
}

int osfx_packet_decode_min(const uint8_t* packet, size_t packet_len, uint64_t out_fields[9]) {
    uint8_t route_count;
    size_t tid_pos;
    size_t body_off;
    size_t body_len;
    uint8_t crc8_recv;
    uint8_t crc8_calc;
    uint16_t crc16_recv;
    uint16_t crc16_calc;
    uint64_t ts;

    if (!packet || !out_fields || packet_len < 16) {
        return 0;
    }

    route_count = packet[1];
    tid_pos = 2U + ((size_t)route_count * 4U);
    if (packet_len < tid_pos + 1U + 6U + 3U) {
        return 0;
    }

    body_off = tid_pos + 7U;
    body_len = packet_len - body_off - 3U;
    crc8_recv = packet[packet_len - 3U];
    crc8_calc = osfx_crc8(packet + body_off, body_len, 0x07U, 0x00U);

    crc16_recv = (uint16_t)(((uint16_t)packet[packet_len - 2U] << 8) | (uint16_t)packet[packet_len - 1U]);
    crc16_calc = osfx_crc16_ccitt(packet, packet_len - 2U, 0x1021U, 0xFFFFU);

    ts = ((uint64_t)packet[tid_pos + 1U] << 40) |
         ((uint64_t)packet[tid_pos + 2U] << 32) |
         ((uint64_t)packet[tid_pos + 3U] << 24) |
         ((uint64_t)packet[tid_pos + 4U] << 16) |
         ((uint64_t)packet[tid_pos + 5U] << 8) |
         (uint64_t)packet[tid_pos + 6U];

    out_fields[0] = (uint64_t)packet[0];
    out_fields[1] = (uint64_t)route_count;
    out_fields[2] = (uint64_t)read_u32_be(packet + 2);
    out_fields[3] = (uint64_t)packet[tid_pos];
    out_fields[4] = ts;
    out_fields[5] = (uint64_t)body_off;
    out_fields[6] = (uint64_t)body_len;
    out_fields[7] = (uint64_t)(crc8_calc == crc8_recv ? 1 : 0);
    out_fields[8] = (uint64_t)(crc16_calc == crc16_recv ? 1 : 0);

    return 1;
}

int osfx_packet_decode_meta(const uint8_t* packet, size_t packet_len, osfx_packet_meta* out_meta) {
    uint64_t f[9];
    if (!out_meta) {
        return 0;
    }
    if (!osfx_packet_decode_min(packet, packet_len, f)) {
        return 0;
    }
    out_meta->cmd = (uint8_t)f[0];
    out_meta->route_count = (uint8_t)f[1];
    out_meta->source_aid = (uint32_t)f[2];
    out_meta->tid = (uint8_t)f[3];
    out_meta->timestamp_raw = f[4];
    out_meta->body_offset = (size_t)f[5];
    out_meta->body_len = (size_t)f[6];
    out_meta->crc8_ok = (int)f[7];
    out_meta->crc16_ok = (int)f[8];
    return 1;
}

int osfx_packet_extract_body(
    const uint8_t* packet,
    size_t packet_len,
    const uint8_t* key,
    size_t key_len,
    uint8_t* out_body,
    size_t out_body_cap,
    size_t* out_body_len,
    osfx_packet_meta* out_meta
) {
    osfx_packet_meta meta;
    const uint8_t* body;
    size_t body_len;
    uint8_t crc8_recv;
    uint8_t crc8_calc;
    int secure;
    if (!packet || !out_body || !out_body_len) {
        return 0;
    }
    if (!osfx_packet_decode_meta(packet, packet_len, &meta)) {
        return 0;
    }
    if (!meta.crc16_ok) {
        return 0;
    }
    body = packet + meta.body_offset;
    body_len = meta.body_len;
    if (body_len > out_body_cap) {
        return 0;
    }
    crc8_recv = packet[packet_len - 3U];
    secure = (meta.cmd != (uint8_t)osfx_cmd_normalize_data(meta.cmd));
    if (secure) {
        osfx_xor_payload(body, body_len, key, key_len, (uint32_t)(crc8_recv & 31U), out_body);
    } else if (body_len > 0U) {
        memcpy(out_body, body, body_len);
    }
    crc8_calc = osfx_crc8(out_body, body_len, 0x07U, 0x00U);
    if (crc8_calc != crc8_recv) {
        return 0;
    }
    *out_body_len = body_len;
    if (out_meta) {
        *out_meta = meta;
        out_meta->crc8_ok = 1;
    }
    return 1;
}

