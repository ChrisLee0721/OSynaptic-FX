#ifndef OSFX_PLATFORM_RUNTIME_H
#define OSFX_PLATFORM_RUNTIME_H

#include <stddef.h>

#include "osfx_plugin_port_forwarder.h"
#include "osfx_plugin_test.h"
#include "osfx_plugin_transport.h"
#include "osfx_service_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct osfx_platform_runtime {
    osfx_service_runtime services;
    osfx_plugin_transport transport;
    osfx_plugin_test test_plugin;
    osfx_plugin_port_forwarder port_forwarder;
} osfx_platform_runtime;

void osfx_platform_runtime_init(
    osfx_platform_runtime* rt,
    osfx_protocol_matrix* matrix,
    osfx_pf_emit_fn pf_emit_fn,
    void* pf_emit_ctx
);

size_t osfx_platform_plugin_count(const osfx_platform_runtime* rt);
int osfx_platform_plugin_name_at(const osfx_platform_runtime* rt, size_t index, char* out_name, size_t out_name_cap);
int osfx_platform_plugin_load(osfx_platform_runtime* rt, const char* name, const char* config);
int osfx_platform_plugin_cmd(osfx_platform_runtime* rt, const char* name, const char* cmd, const char* args, char* out, size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif

