#ifndef OSFX_TRANSPORTER_RUNTIME_H
#define OSFX_TRANSPORTER_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OSFX_TRANSPORTER_MAX 16
#define OSFX_TRANSPORTER_NAME_MAX 24

typedef int (*osfx_send_fn)(const uint8_t* payload, size_t payload_len, void* user_ctx);

typedef struct osfx_transporter_driver {
    char name[OSFX_TRANSPORTER_NAME_MAX];
    int priority;
    int enabled;
    osfx_send_fn send_fn;
    void* user_ctx;
    int max_retries;
    int used;
} osfx_transporter_driver;

typedef struct osfx_transporter_runtime {
    osfx_transporter_driver drivers[OSFX_TRANSPORTER_MAX];
} osfx_transporter_runtime;

void osfx_transporter_runtime_init(osfx_transporter_runtime* rt);
int osfx_transporter_register(
    osfx_transporter_runtime* rt,
    const char* name,
    int priority,
    osfx_send_fn send_fn,
    void* user_ctx
);
int osfx_transporter_set_enabled(osfx_transporter_runtime* rt, const char* name, int enabled);
int osfx_transporter_set_retry(osfx_transporter_runtime* rt, const char* name, int max_retries);
int osfx_transporter_dispatch_named(
    osfx_transporter_runtime* rt,
    const char* name,
    const uint8_t* payload,
    size_t payload_len
);
int osfx_transporter_dispatch_auto(
    osfx_transporter_runtime* rt,
    const uint8_t* payload,
    size_t payload_len,
    char* out_used_name,
    size_t out_used_name_cap
);

#ifdef __cplusplus
}
#endif

#endif

