#include <stdio.h>
#include <string.h>

#include "osfx_core.h"
#include "osfx_build_config.h"

typedef struct cli_emit_ctx {
    int emitted;
    char last_proto[16];
    uint16_t last_port;
    size_t last_len;
} cli_emit_ctx;

static int matrix_emit(const char* protocol, const uint8_t* frame, size_t frame_len, void* user_ctx) {
    cli_emit_ctx* ctx = (cli_emit_ctx*)user_ctx;
    (void)frame;
    if (!ctx || !protocol) {
        return 0;
    }
    ctx->emitted++;
    strncpy(ctx->last_proto, protocol, sizeof(ctx->last_proto) - 1U);
    ctx->last_proto[sizeof(ctx->last_proto) - 1U] = '\0';
    ctx->last_len = frame_len;
    return 1;
}

static int pf_emit(const char* to_proto, uint16_t to_port, const uint8_t* payload, size_t payload_len, void* user_ctx) {
    cli_emit_ctx* ctx = (cli_emit_ctx*)user_ctx;
    (void)payload;
    if (!ctx || !to_proto) {
        return 0;
    }
    ctx->emitted++;
    strncpy(ctx->last_proto, to_proto, sizeof(ctx->last_proto) - 1U);
    ctx->last_proto[sizeof(ctx->last_proto) - 1U] = '\0';
    ctx->last_port = to_port;
    ctx->last_len = payload_len;
    return 1;
}

int main(int argc, char** argv) {
    osfx_protocol_matrix pm;
    osfx_platform_runtime platform;
    cli_emit_ctx emit_ctx;
    char out[1024];
    int ok;

    memset(&emit_ctx, 0, sizeof(emit_ctx));
    osfx_protocol_matrix_init(&pm, matrix_emit, &emit_ctx);
    if (!osfx_protocol_matrix_register_defaults(&pm)) {
        fprintf(stderr, "error=protocol_matrix_init_failed\n");
        return 1;
    }
    osfx_platform_runtime_init(&platform, &pm, pf_emit, &emit_ctx);
    #if OSFX_CFG_AUTOLOAD_TRANSPORT
    osfx_platform_plugin_load(&platform, "transport", "");
    #endif
    #if OSFX_CFG_AUTOLOAD_TEST_PLUGIN
    osfx_platform_plugin_load(&platform, "test_plugin", "");
    #endif
    #if OSFX_CFG_AUTOLOAD_PORT_FORWARDER
    osfx_platform_plugin_load(&platform, "port_forwarder", "");
    #endif

    ok = osfx_cli_lite_run(&platform, argc - 1, (const char**)(argv + 1), out, sizeof(out));
    puts(out);
    return ok ? 0 : 1;
}

