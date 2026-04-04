#ifndef OSFX_SECURE_SESSION_H
#define OSFX_SECURE_SESSION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OSFX_SESSION_MAX 64
#define OSFX_KEY_LEN 32

enum {
    OSFX_SEC_STATE_INIT = 0,
    OSFX_SEC_STATE_PLAINTEXT_SENT = 1,
    OSFX_SEC_STATE_DICT_READY = 2,
    OSFX_SEC_STATE_SECURE = 3
};

enum {
    OSFX_TS_ACCEPT = 0,
    OSFX_TS_REPLAY = 1,
    OSFX_TS_OUT_OF_ORDER = 2
};

typedef struct osfx_secure_session {
    uint32_t aid;
    uint64_t last_seen;
    uint64_t last_data_timestamp;
    uint64_t first_plaintext_ts;
    uint64_t pending_timestamp;
    uint8_t key[OSFX_KEY_LEN];
    uint8_t pending_key[OSFX_KEY_LEN];
    int key_set;
    int pending_key_set;
    int dict_ready;
    int decrypt_confirmed;
    int state;
    int used;
} osfx_secure_session;

typedef struct osfx_secure_session_store {
    osfx_secure_session sessions[OSFX_SESSION_MAX];
    uint64_t expire_seconds;
} osfx_secure_session_store;

void osfx_secure_store_init(osfx_secure_session_store* store, uint64_t expire_seconds);
void osfx_secure_store_cleanup(osfx_secure_session_store* store, uint64_t now_ts);

int osfx_secure_note_plaintext_sent(osfx_secure_session_store* store, uint32_t aid, uint64_t timestamp_raw, uint64_t now_ts);
int osfx_secure_confirm_dict(osfx_secure_session_store* store, uint32_t aid, uint64_t timestamp_raw, uint64_t now_ts);
int osfx_secure_mark_channel(osfx_secure_session_store* store, uint32_t aid, uint64_t now_ts);
int osfx_secure_should_encrypt(const osfx_secure_session_store* store, uint32_t aid);
int osfx_secure_get_key(const osfx_secure_session_store* store, uint32_t aid, uint8_t out_key[OSFX_KEY_LEN]);
int osfx_secure_is_channel_confirmed(const osfx_secure_session_store* store, uint32_t aid);
int osfx_secure_check_and_update_timestamp(osfx_secure_session_store* store, uint32_t aid, uint64_t timestamp_raw, uint64_t now_ts);

int osfx_secure_store_save(const osfx_secure_session_store* store, const char* path);
int osfx_secure_store_load(osfx_secure_session_store* store, const char* path);

#ifdef __cplusplus
}
#endif

#endif

