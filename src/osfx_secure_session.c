#include "../include/osfx_secure_session.h"
#include "../include/osfx_build_config.h"

#if OSFX_ENABLE_FILE_IO
#include <stdio.h>
#endif
#include <string.h>

static osfx_secure_session* find_or_alloc(osfx_secure_session_store* store, uint32_t aid) {
    size_t i;
    osfx_secure_session* empty = NULL;
    if (!store) {
        return NULL;
    }
    for (i = 0; i < OSFX_SESSION_MAX; ++i) {
        if (store->sessions[i].used && store->sessions[i].aid == aid) {
            return &store->sessions[i];
        }
        if (!store->sessions[i].used && !empty) {
            empty = &store->sessions[i];
        }
    }
    if (empty) {
        memset(empty, 0, sizeof(*empty));
        empty->used = 1;
        empty->aid = aid;
        empty->state = OSFX_SEC_STATE_INIT;
    }
    return empty;
}

static const osfx_secure_session* find_const(const osfx_secure_session_store* store, uint32_t aid) {
    size_t i;
    if (!store) {
        return NULL;
    }
    for (i = 0; i < OSFX_SESSION_MAX; ++i) {
        if (store->sessions[i].used && store->sessions[i].aid == aid) {
            return &store->sessions[i];
        }
    }
    return NULL;
}

static void derive_key(uint32_t aid, uint64_t ts, uint8_t out_key[OSFX_KEY_LEN]) {
    size_t i;
    uint32_t x = aid ^ (uint32_t)(ts & 0xFFFFFFFFU) ^ (uint32_t)((ts >> 32) & 0xFFFFFFFFU);
    for (i = 0; i < OSFX_KEY_LEN; ++i) {
        x = (x * 1664525U) + 1013904223U + (uint32_t)i;
        out_key[i] = (uint8_t)((x >> 24) & 0xFFU);
    }
}

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return (int)(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (int)(c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + (int)(c - 'A');
    }
    return -1;
}

static void bytes_to_hex(const uint8_t* in, size_t n, char* out, size_t out_cap) {
    static const char HEX[] = "0123456789abcdef";
    size_t i;
    if (!in || !out || out_cap < (n * 2U + 1U)) {
        return;
    }
    for (i = 0; i < n; ++i) {
        out[2U * i] = HEX[(in[i] >> 4) & 0x0FU];
        out[2U * i + 1U] = HEX[in[i] & 0x0FU];
    }
    out[n * 2U] = '\0';
}

static int hex_to_bytes(const char* text, uint8_t* out, size_t n) {
    size_t i;
    if (!text || !out) {
        return 0;
    }
    for (i = 0; i < n; ++i) {
        int hi = hex_nibble(text[2U * i]);
        int lo = hex_nibble(text[2U * i + 1U]);
        if (hi < 0 || lo < 0) {
            return 0;
        }
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return 1;
}

void osfx_secure_store_init(osfx_secure_session_store* store, uint64_t expire_seconds) {
    if (!store) {
        return;
    }
    memset(store, 0, sizeof(*store));
    store->expire_seconds = (expire_seconds > 0U) ? expire_seconds : 86400U;
}

void osfx_secure_store_cleanup(osfx_secure_session_store* store, uint64_t now_ts) {
    size_t i;
    if (!store) {
        return;
    }
    for (i = 0; i < OSFX_SESSION_MAX; ++i) {
        if (!store->sessions[i].used) {
            continue;
        }
        if (now_ts > store->sessions[i].last_seen && (now_ts - store->sessions[i].last_seen) > store->expire_seconds) {
            memset(&store->sessions[i], 0, sizeof(store->sessions[i]));
        }
    }
}

int osfx_secure_note_plaintext_sent(osfx_secure_session_store* store, uint32_t aid, uint64_t timestamp_raw, uint64_t now_ts) {
    osfx_secure_session* s = find_or_alloc(store, aid);
    if (!s) {
        return 0;
    }
    s->last_seen = now_ts;
    s->pending_timestamp = timestamp_raw;
    derive_key(aid, timestamp_raw, s->pending_key);
    s->pending_key_set = 1;
    s->state = OSFX_SEC_STATE_PLAINTEXT_SENT;
    return 1;
}

int osfx_secure_confirm_dict(osfx_secure_session_store* store, uint32_t aid, uint64_t timestamp_raw, uint64_t now_ts) {
    osfx_secure_session* s = find_or_alloc(store, aid);
    if (!s) {
        return 0;
    }
    s->last_seen = now_ts;
    if (!s->pending_key_set) {
        s->pending_timestamp = timestamp_raw;
        derive_key(aid, timestamp_raw, s->pending_key);
        s->pending_key_set = 1;
    }
    memcpy(s->key, s->pending_key, OSFX_KEY_LEN);
    s->key_set = 1;
    s->dict_ready = 1;
    s->first_plaintext_ts = s->pending_timestamp;
    s->state = OSFX_SEC_STATE_DICT_READY;
    return 1;
}

int osfx_secure_mark_channel(osfx_secure_session_store* store, uint32_t aid, uint64_t now_ts) {
    osfx_secure_session* s = find_or_alloc(store, aid);
    if (!s) {
        return 0;
    }
    s->last_seen = now_ts;
    s->decrypt_confirmed = 1;
    s->state = OSFX_SEC_STATE_SECURE;
    return 1;
}

int osfx_secure_should_encrypt(const osfx_secure_session_store* store, uint32_t aid) {
    const osfx_secure_session* s = find_const(store, aid);
    if (!s) {
        return 0;
    }
    return (s->dict_ready && s->key_set) ? 1 : 0;
}

int osfx_secure_get_key(const osfx_secure_session_store* store, uint32_t aid, uint8_t out_key[OSFX_KEY_LEN]) {
    const osfx_secure_session* s = find_const(store, aid);
    if (!s || !out_key || !s->key_set) {
        return 0;
    }
    memcpy(out_key, s->key, OSFX_KEY_LEN);
    return 1;
}

int osfx_secure_is_channel_confirmed(const osfx_secure_session_store* store, uint32_t aid) {
    const osfx_secure_session* s = find_const(store, aid);
    if (!s) {
        return 0;
    }
    return s->decrypt_confirmed ? 1 : 0;
}

int osfx_secure_check_and_update_timestamp(osfx_secure_session_store* store, uint32_t aid, uint64_t timestamp_raw, uint64_t now_ts) {
    osfx_secure_session* s = find_or_alloc(store, aid);
    if (!s) {
        return OSFX_TS_OUT_OF_ORDER;
    }
    s->last_seen = now_ts;
    if (s->last_data_timestamp == 0U) {
        s->last_data_timestamp = timestamp_raw;
        return OSFX_TS_ACCEPT;
    }
    if (timestamp_raw == s->last_data_timestamp) {
        return OSFX_TS_REPLAY;
    }
    if (timestamp_raw < s->last_data_timestamp) {
        return OSFX_TS_OUT_OF_ORDER;
    }
    s->last_data_timestamp = timestamp_raw;
    return OSFX_TS_ACCEPT;
}

int osfx_secure_store_save(const osfx_secure_session_store* store, const char* path) {
#if !OSFX_ENABLE_FILE_IO
    (void)store;
    (void)path;
    return 0;
#else
    FILE* fp;
    size_t i;
    if (!store || !path) {
        return 0;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    for (i = 0; i < OSFX_SESSION_MAX; ++i) {
        if (store->sessions[i].used) {
            char key_hex[OSFX_KEY_LEN * 2 + 1];
            char pend_hex[OSFX_KEY_LEN * 2 + 1];
            bytes_to_hex(store->sessions[i].key, OSFX_KEY_LEN, key_hex, sizeof(key_hex));
            bytes_to_hex(store->sessions[i].pending_key, OSFX_KEY_LEN, pend_hex, sizeof(pend_hex));
            fprintf(fp, "%u,%llu,%llu,%llu,%llu,%d,%d,%d,%d,%d,%s,%s\n",
                    (unsigned)store->sessions[i].aid,
                    (unsigned long long)store->sessions[i].last_seen,
                    (unsigned long long)store->sessions[i].last_data_timestamp,
                    (unsigned long long)store->sessions[i].first_plaintext_ts,
                    (unsigned long long)store->sessions[i].pending_timestamp,
                    store->sessions[i].key_set,
                    store->sessions[i].pending_key_set,
                    store->sessions[i].dict_ready,
                    store->sessions[i].decrypt_confirmed,
                    store->sessions[i].state,
                    key_hex,
                    pend_hex);
        }
    }
    fclose(fp);
    return 1;
#endif
}

int osfx_secure_store_load(osfx_secure_session_store* store, const char* path) {
#if !OSFX_ENABLE_FILE_IO
    (void)store;
    (void)path;
    return 0;
#else
    FILE* fp;
    char line[512];
    if (!store || !path) {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    memset(store->sessions, 0, sizeof(store->sessions));
    while (fgets(line, sizeof(line), fp)) {
        unsigned aid = 0;
        unsigned long long last = 0;
        unsigned long long last_data_ts = 0;
        unsigned long long first_ts = 0;
        unsigned long long pending_ts = 0;
        int key_set = 0;
        int pending_set = 0;
        int dict_ready = 0;
        int decrypt = 0;
        int state = OSFX_SEC_STATE_INIT;
        char key_hex[OSFX_KEY_LEN * 2 + 1];
        char pend_hex[OSFX_KEY_LEN * 2 + 1];
        osfx_secure_session* s;
        int got = sscanf(line, "%u,%llu,%llu,%llu,%llu,%d,%d,%d,%d,%d,%64[^,],%64s",
                         &aid, &last, &last_data_ts, &first_ts, &pending_ts,
                         &key_set, &pending_set, &dict_ready, &decrypt, &state,
                         key_hex, pend_hex);
        if (got != 12) {
            got = sscanf(line, "%u,%llu,%llu,%llu,%d,%d,%d,%d,%64[^,],%64s",
                         &aid, &last, &first_ts, &pending_ts,
                         &key_set, &pending_set, &dict_ready, &decrypt,
                         key_hex, pend_hex);
            if (got != 10) {
                continue;
            }
            last_data_ts = 0;
            state = dict_ready ? OSFX_SEC_STATE_DICT_READY : OSFX_SEC_STATE_INIT;
        }
        s = find_or_alloc(store, (uint32_t)aid);
        if (!s) {
            continue;
        }
        s->last_seen = (uint64_t)last;
        s->last_data_timestamp = (uint64_t)last_data_ts;
        s->first_plaintext_ts = (uint64_t)first_ts;
        s->pending_timestamp = (uint64_t)pending_ts;
        s->key_set = key_set ? 1 : 0;
        s->pending_key_set = pending_set ? 1 : 0;
        s->dict_ready = dict_ready ? 1 : 0;
        s->decrypt_confirmed = decrypt ? 1 : 0;
        s->state = state;
        if (!hex_to_bytes(key_hex, s->key, OSFX_KEY_LEN)) {
            memset(s->key, 0, OSFX_KEY_LEN);
            s->key_set = 0;
        }
        if (!hex_to_bytes(pend_hex, s->pending_key, OSFX_KEY_LEN)) {
            memset(s->pending_key, 0, OSFX_KEY_LEN);
            s->pending_key_set = 0;
        }
    }
    fclose(fp);
    return 1;
#endif
}

