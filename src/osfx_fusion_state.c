#include "../include/osfx_fusion_state.h"

#include <string.h>

#include "../include/osfx_fusion_packet.h"

static int split_prefix_value(const uint8_t* body, size_t body_len,
                              char* out_prefix, size_t out_prefix_cap, size_t* out_prefix_len,
                              char* out_value, size_t out_value_cap, size_t* out_value_len) {
    size_t i;
    size_t end = body_len;
    size_t last = (size_t)-1;
    if (!body || body_len == 0 || !out_prefix || !out_value || !out_prefix_len || !out_value_len) {
        return 0;
    }
    while (end > 0 && body[end - 1] == '|') {
        --end;
    }
    if (end == 0) {
        return 0;
    }

    for (i = 0; i < end; ++i) {
        if (body[i] == '|') {
            last = i;
        }
    }
    if (last == (size_t)-1 || last + 1 >= end) {
        return 0;
    }
    if (last + 1 > out_prefix_cap || end - (last + 1) + 1 > out_value_cap) {
        return 0;
    }

    memcpy(out_prefix, body, last + 1);
    out_prefix[last + 1] = '\0';
    *out_prefix_len = last + 1;

    memcpy(out_value, body + last + 1, end - (last + 1));
    out_value[end - (last + 1)] = '\0';
    *out_value_len = end - (last + 1);
    return 1;
}

static osfx_fusion_entry* find_entry(osfx_fusion_state* st, uint32_t aid, uint8_t tid) {
    size_t i;
    if (!st) {
        return NULL;
    }
    for (i = 0; i < OSFX_FUSION_MAX_ENTRIES; ++i) {
        if (st->entries[i].used && st->entries[i].source_aid == aid && st->entries[i].tid == tid) {
            return &st->entries[i];
        }
    }
    return NULL;
}

static osfx_fusion_entry* alloc_entry(osfx_fusion_state* st, uint32_t aid, uint8_t tid) {
    size_t i;
    if (!st) {
        return NULL;
    }
    for (i = 0; i < OSFX_FUSION_MAX_ENTRIES; ++i) {
        if (!st->entries[i].used) {
            st->entries[i].used = 1;
            st->entries[i].source_aid = aid;
            st->entries[i].tid = tid;
            st->entries[i].prefix[0] = '\0';
            st->entries[i].prefix_len = 0;
            st->entries[i].last_value[0] = '\0';
            st->entries[i].last_value_len = 0;
            return &st->entries[i];
        }
    }
    return NULL;
}

void osfx_fusion_state_init(osfx_fusion_state* st) {
    if (st) {
        memset(st, 0, sizeof(*st));
    }
}

void osfx_fusion_state_reset(osfx_fusion_state* st) {
    osfx_fusion_state_init(st);
}

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
) {
    osfx_fusion_entry* e;
    char prefix[OSFX_FUSION_MAX_PREFIX];
    char value[OSFX_FUSION_MAX_VALUE];
    size_t prefix_len = 0;
    size_t value_len = 0;
    int pkt_len;
    uint8_t cmd;
    const uint8_t* body_to_send;
    size_t body_to_send_len;

    if (!st || !full_body || full_body_len == 0 || !out_packet || !out_packet_len) {
        return 0;
    }
    if (!split_prefix_value(full_body, full_body_len, prefix, sizeof(prefix), &prefix_len, value, sizeof(value), &value_len)) {
        /*
         * Large/extended bodies may exceed fusion cache token limits.
         * In that case, degrade to FULL packet without state caching.
         */
        pkt_len = osfx_packet_encode_full(
            (uint8_t)OSFX_CMD_DATA_FULL,
            source_aid,
            tid,
            timestamp_raw,
            full_body,
            full_body_len,
            out_packet,
            out_packet_cap
        );
        if (pkt_len <= 0) {
            return 0;
        }
        *out_packet_len = pkt_len;
        if (out_cmd) {
            *out_cmd = (uint8_t)OSFX_CMD_DATA_FULL;
        }
        return 1;
    }

    e = find_entry(st, source_aid, tid);
    if (!e) {
        e = alloc_entry(st, source_aid, tid);
    }
    if (!e) {
        return 0;
    }

    if (e->prefix_len == 0 || e->last_value_len == 0 || e->prefix_len != prefix_len || memcmp(e->prefix, prefix, prefix_len) != 0) {
        cmd = (uint8_t)OSFX_CMD_DATA_FULL;
        body_to_send = full_body;
        body_to_send_len = full_body_len;
        memcpy(e->prefix, prefix, prefix_len + 1);
        e->prefix_len = prefix_len;
        memcpy(e->last_value, value, value_len + 1);
        e->last_value_len = value_len;
    } else if (e->last_value_len == value_len && memcmp(e->last_value, value, value_len) == 0) {
        cmd = (uint8_t)OSFX_CMD_DATA_HEART;
        body_to_send = NULL;
        body_to_send_len = 0;
    } else {
        cmd = (uint8_t)OSFX_CMD_DATA_DIFF;
        body_to_send = (const uint8_t*)value;
        body_to_send_len = value_len;
        memcpy(e->last_value, value, value_len + 1);
        e->last_value_len = value_len;
    }

    pkt_len = osfx_packet_encode_full(cmd, source_aid, tid, timestamp_raw, body_to_send, body_to_send_len, out_packet, out_packet_cap);
    if (pkt_len <= 0) {
        return 0;
    }
    *out_packet_len = pkt_len;
    if (out_cmd) {
        *out_cmd = cmd;
    }
    return 1;
}

int osfx_fusion_decode_apply(
    osfx_fusion_state* st,
    const uint8_t* packet,
    size_t packet_len,
    uint8_t* out_body,
    size_t out_body_cap,
    size_t* out_body_len,
    osfx_packet_meta* out_meta
) {
    osfx_packet_meta meta;
    osfx_fusion_entry* e;
    const uint8_t* body;
    size_t body_len;

    if (!st || !packet || !out_body || !out_body_len) {
        return 0;
    }
    if (!osfx_packet_decode_meta(packet, packet_len, &meta)) {
        return 0;
    }
    body = packet + meta.body_offset;
    body_len = meta.body_len;

    e = find_entry(st, meta.source_aid, meta.tid);
    if (!e && meta.cmd != (uint8_t)OSFX_CMD_DATA_FULL) {
        return 0;
    }

    if (meta.cmd == (uint8_t)OSFX_CMD_DATA_FULL) {
        char prefix[OSFX_FUSION_MAX_PREFIX];
        char value[OSFX_FUSION_MAX_VALUE];
        size_t prefix_len = 0;
        size_t value_len = 0;
        if (!split_prefix_value(body, body_len, prefix, sizeof(prefix), &prefix_len, value, sizeof(value), &value_len)) {
            /*
             * For oversized FULL bodies, decode still succeeds but no DIFF/HEART cache is updated.
             */
            if (body_len > out_body_cap) {
                return 0;
            }
            memcpy(out_body, body, body_len);
            *out_body_len = body_len;
            if (out_meta) {
                *out_meta = meta;
            }
            return 1;
        }
        if (!e) {
            e = alloc_entry(st, meta.source_aid, meta.tid);
            if (!e) {
                return 0;
            }
        }
        memcpy(e->prefix, prefix, prefix_len + 1);
        e->prefix_len = prefix_len;
        memcpy(e->last_value, value, value_len + 1);
        e->last_value_len = value_len;

        if (body_len > out_body_cap) {
            return 0;
        }
        memcpy(out_body, body, body_len);
        *out_body_len = body_len;
    } else if (meta.cmd == (uint8_t)OSFX_CMD_DATA_DIFF) {
        if (!e || e->prefix_len == 0 || body_len == 0) {
            return 0;
        }
        if (e->prefix_len + body_len > out_body_cap) {
            return 0;
        }
        memcpy(out_body, e->prefix, e->prefix_len);
        memcpy(out_body + e->prefix_len, body, body_len);
        *out_body_len = e->prefix_len + body_len;

        if (body_len + 1 > sizeof(e->last_value)) {
            return 0;
        }
        memcpy(e->last_value, body, body_len);
        e->last_value[body_len] = '\0';
        e->last_value_len = body_len;
    } else if (meta.cmd == (uint8_t)OSFX_CMD_DATA_HEART) {
        if (!e || e->prefix_len == 0 || e->last_value_len == 0) {
            return 0;
        }
        if (e->prefix_len + e->last_value_len > out_body_cap) {
            return 0;
        }
        memcpy(out_body, e->prefix, e->prefix_len);
        memcpy(out_body + e->prefix_len, e->last_value, e->last_value_len);
        *out_body_len = e->prefix_len + e->last_value_len;
    } else {
        return 0;
    }

    if (out_meta) {
        *out_meta = meta;
    }
    return 1;
}

