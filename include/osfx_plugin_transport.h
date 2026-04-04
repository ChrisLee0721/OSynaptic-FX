#ifndef OSFX_PLUGIN_TRANSPORT_H
#define OSFX_PLUGIN_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

#include "osfx_protocol_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct osfx_plugin_transport {
    osfx_protocol_matrix* matrix;
    int initialized;
    uint64_t dispatch_count;
    char last_proto[16];
    uint64_t last_payload_len;
} osfx_plugin_transport;

void osfx_plugin_transport_init_ctx(osfx_plugin_transport* plugin, osfx_protocol_matrix* matrix);

int osfx_plugin_transport_init(void* instance, const char* config);
int osfx_plugin_transport_tick(void* instance, uint64_t now_ts);
int osfx_plugin_transport_close(void* instance);
int osfx_plugin_transport_reload(void* instance, const char* config);
int osfx_plugin_transport_command(void* instance, const char* cmd, const char* args, char* out, size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif

