#ifndef OSFX_SERVICE_RUNTIME_H
#define OSFX_SERVICE_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OSFX_SERVICE_MAX 32
#define OSFX_SERVICE_NAME_MAX 32
#define OSFX_SERVICE_CONFIG_MAX 512

typedef int (*osfx_service_init_fn)(void* instance, const char* config);
typedef int (*osfx_service_tick_fn)(void* instance, uint64_t now_ts);
typedef int (*osfx_service_close_fn)(void* instance);
typedef int (*osfx_service_reload_fn)(void* instance, const char* config);
typedef int (*osfx_service_cmd_fn)(void* instance, const char* cmd, const char* args, char* out, size_t out_cap);

typedef struct osfx_service_slot {
    char name[OSFX_SERVICE_NAME_MAX];
    char config[OSFX_SERVICE_CONFIG_MAX];
    void* instance;
    osfx_service_init_fn init_fn;
    osfx_service_tick_fn tick_fn;
    osfx_service_close_fn close_fn;
    osfx_service_reload_fn reload_fn;
    osfx_service_cmd_fn cmd_fn;
    int used;
    int enabled;
    int running;
    int last_rc;
    uint32_t init_count;
    uint32_t tick_count;
    uint32_t close_count;
    uint32_t reload_count;
    uint32_t cmd_count;
} osfx_service_slot;

typedef struct osfx_service_runtime {
    osfx_service_slot services[OSFX_SERVICE_MAX];
} osfx_service_runtime;

typedef struct osfx_service_status {
    int enabled;
    int running;
    int last_rc;
    uint32_t init_count;
    uint32_t tick_count;
    uint32_t close_count;
    uint32_t reload_count;
    uint32_t cmd_count;
} osfx_service_status;

void osfx_service_runtime_init(osfx_service_runtime* rt);

int osfx_service_register(
    osfx_service_runtime* rt,
    const char* name,
    void* instance,
    osfx_service_init_fn init_fn,
    osfx_service_tick_fn tick_fn,
    osfx_service_close_fn close_fn,
    osfx_service_reload_fn reload_fn,
    const char* initial_config
);

int osfx_service_register_ex(
    osfx_service_runtime* rt,
    const char* name,
    void* instance,
    osfx_service_init_fn init_fn,
    osfx_service_tick_fn tick_fn,
    osfx_service_close_fn close_fn,
    osfx_service_reload_fn reload_fn,
    osfx_service_cmd_fn cmd_fn,
    const char* initial_config
);

int osfx_service_set_enabled(osfx_service_runtime* rt, const char* name, int enabled);
int osfx_service_set_config(osfx_service_runtime* rt, const char* name, const char* config);
const char* osfx_service_get_config(const osfx_service_runtime* rt, const char* name);

int osfx_service_init_all(osfx_service_runtime* rt, const char* default_config);
int osfx_service_tick_all(osfx_service_runtime* rt, uint64_t now_ts);
int osfx_service_reload(osfx_service_runtime* rt, const char* name, const char* config);
int osfx_service_close_all(osfx_service_runtime* rt);
int osfx_service_get_status(const osfx_service_runtime* rt, const char* name, osfx_service_status* out_status);

size_t osfx_service_count(const osfx_service_runtime* rt);
int osfx_service_name_at(const osfx_service_runtime* rt, size_t index, char* out_name, size_t out_name_cap);
int osfx_service_load(osfx_service_runtime* rt, const char* name, const char* default_config);
int osfx_service_command(osfx_service_runtime* rt, const char* name, const char* cmd, const char* args, char* out, size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif

