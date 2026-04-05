#include "../include/osfx_platform_runtime.h"
#include "../include/osfx_build_config.h"

#include <string.h>

void osfx_platform_runtime_init(
    osfx_platform_runtime* rt,
    osfx_protocol_matrix* matrix,
    osfx_pf_emit_fn pf_emit_fn,
    void* pf_emit_ctx
) {
    if (!rt) {
        return;
    }
    memset(rt, 0, sizeof(*rt));
    osfx_service_runtime_init(&rt->services);

    osfx_plugin_transport_init_ctx(&rt->transport, matrix);
    osfx_plugin_test_init_ctx(&rt->test_plugin);
    osfx_plugin_port_forwarder_init_ctx(&rt->port_forwarder, pf_emit_fn, pf_emit_ctx);

    #if OSFX_CFG_PLUGIN_TRANSPORT
    osfx_service_register_ex(
        &rt->services,
        "transport",
        &rt->transport,
        osfx_plugin_transport_init,
        osfx_plugin_transport_tick,
        osfx_plugin_transport_close,
        osfx_plugin_transport_reload,
        osfx_plugin_transport_command,
        ""
    );
    #endif
    #if OSFX_CFG_PLUGIN_TEST_PLUGIN
    osfx_service_register_ex(
        &rt->services,
        "test_plugin",
        &rt->test_plugin,
        osfx_plugin_test_init,
        osfx_plugin_test_tick,
        osfx_plugin_test_close,
        osfx_plugin_test_reload,
        osfx_plugin_test_command,
        ""
    );
    #endif
    #if OSFX_CFG_PLUGIN_PORT_FORWARDER
    osfx_service_register_ex(
        &rt->services,
        "port_forwarder",
        &rt->port_forwarder,
        osfx_plugin_port_forwarder_init,
        osfx_plugin_port_forwarder_tick,
        osfx_plugin_port_forwarder_close,
        osfx_plugin_port_forwarder_reload,
        osfx_plugin_port_forwarder_command,
        ""
    );
    #endif
}

size_t osfx_platform_plugin_count(const osfx_platform_runtime* rt) {
    return osfx_service_count(rt ? &rt->services : NULL);
}

int osfx_platform_plugin_name_at(const osfx_platform_runtime* rt, size_t index, char* out_name, size_t out_name_cap) {
    return osfx_service_name_at(rt ? &rt->services : NULL, index, out_name, out_name_cap);
}

int osfx_platform_plugin_load(osfx_platform_runtime* rt, const char* name, const char* config) {
    return osfx_service_load(rt ? &rt->services : NULL, name, config);
}

int osfx_platform_plugin_cmd(osfx_platform_runtime* rt, const char* name, const char* cmd, const char* args, char* out, size_t out_cap) {
    return osfx_service_command(rt ? &rt->services : NULL, name, cmd, args, out, out_cap);
}

