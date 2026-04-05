#ifndef OSFX_PLUGIN_PORT_FORWARDER_H
#define OSFX_PLUGIN_PORT_FORWARDER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OSFX_PF_RULE_MAX 64
#define OSFX_PF_NAME_MAX 32
#define OSFX_PF_PROTO_MAX 16
#define OSFX_PF_PATH_MAX 260

typedef int (*osfx_pf_emit_fn)(const char* to_proto, uint16_t to_port, const uint8_t* payload, size_t payload_len, void* user_ctx);

typedef struct osfx_pf_rule {
    char name[OSFX_PF_NAME_MAX];
    char from_proto[OSFX_PF_PROTO_MAX];
    uint16_t from_port;
    int has_from_port;
    char to_proto[OSFX_PF_PROTO_MAX];
    uint16_t to_port;
    uint64_t hit_count;
    int enabled;
    int used;
} osfx_pf_rule;

typedef struct osfx_plugin_port_forwarder {
    osfx_pf_rule rules[OSFX_PF_RULE_MAX];
    char persist_path[OSFX_PF_PATH_MAX];
    uint64_t total_forwarded;
    uint64_t total_dropped;
    int initialized;
    osfx_pf_emit_fn emit_fn;
    void* emit_ctx;
} osfx_plugin_port_forwarder;

void osfx_plugin_port_forwarder_init_ctx(osfx_plugin_port_forwarder* plugin, osfx_pf_emit_fn emit_fn, void* emit_ctx);

int osfx_plugin_port_forwarder_init(void* instance, const char* config);
int osfx_plugin_port_forwarder_tick(void* instance, uint64_t now_ts);
int osfx_plugin_port_forwarder_close(void* instance);
int osfx_plugin_port_forwarder_reload(void* instance, const char* config);
int osfx_plugin_port_forwarder_command(void* instance, const char* cmd, const char* args, char* out, size_t out_cap);

int osfx_pf_add_rule(
    osfx_plugin_port_forwarder* plugin,
    const char* name,
    const char* from_proto,
    const char* from_port_token,
    const char* to_proto,
    uint16_t to_port
);
int osfx_pf_remove_rule(osfx_plugin_port_forwarder* plugin, const char* name);
int osfx_pf_set_rule_enabled(osfx_plugin_port_forwarder* plugin, const char* name, int enabled);
int osfx_pf_forward(
    osfx_plugin_port_forwarder* plugin,
    const char* from_proto,
    uint16_t from_port,
    const uint8_t* payload,
    size_t payload_len,
    char* out_route,
    size_t out_route_cap
);
int osfx_pf_save(const osfx_plugin_port_forwarder* plugin, const char* path);
int osfx_pf_load(osfx_plugin_port_forwarder* plugin, const char* path);

#ifdef __cplusplus
}
#endif

#endif

