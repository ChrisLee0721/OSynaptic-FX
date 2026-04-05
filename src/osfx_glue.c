#include "../include/osfx_glue.h"
#include "../include/osfx_build_config.h"

#include <string.h>

void osfx_glue_default_config(osfx_glue_config* out_cfg) {
    if (!out_cfg) {
        return;
    }
    memset(out_cfg, 0, sizeof(*out_cfg));
    out_cfg->local_aid = 1U;
    out_cfg->id_start = 100U;
    out_cfg->id_end = 10000U;
    out_cfg->id_default_lease_seconds = 86400U;
    out_cfg->secure_expire_seconds = 86400U;
}

int osfx_glue_init(osfx_glue_ctx* ctx, const osfx_glue_config* cfg) {
    osfx_glue_config local_cfg;
    if (!ctx) {
        return OSFX_GLUE_ERR_ARG;
    }
    osfx_glue_default_config(&local_cfg);
    if (cfg) {
        local_cfg = *cfg;
        if (local_cfg.id_start == 0U) {
            local_cfg.id_start = 100U;
        }
        if (local_cfg.id_end < local_cfg.id_start) {
            local_cfg.id_end = local_cfg.id_start;
        }
        if (local_cfg.id_default_lease_seconds == 0U) {
            local_cfg.id_default_lease_seconds = 86400U;
        }
        if (local_cfg.secure_expire_seconds == 0U) {
            local_cfg.secure_expire_seconds = 86400U;
        }
        if (local_cfg.local_aid == 0U) {
            local_cfg.local_aid = local_cfg.id_start;
        }
    }

    memset(ctx, 0, sizeof(*ctx));
    ctx->local_aid = local_cfg.local_aid;
    osfx_fusion_state_init(&ctx->tx_state);
    osfx_fusion_state_init(&ctx->rx_state);
    osfx_secure_store_init(&ctx->secure_store, local_cfg.secure_expire_seconds);
    osfx_id_allocator_init(
        &ctx->id_allocator,
        local_cfg.id_start,
        local_cfg.id_end,
        local_cfg.id_default_lease_seconds
    );
    osfx_protocol_matrix_init(&ctx->protocol_matrix, local_cfg.matrix_emit_fn, local_cfg.matrix_emit_ctx);
    if (!osfx_protocol_matrix_register_defaults(&ctx->protocol_matrix)) {
        return OSFX_GLUE_ERR_RUNTIME;
    }
    osfx_platform_runtime_init(&ctx->platform_runtime, &ctx->protocol_matrix, local_cfg.pf_emit_fn, local_cfg.pf_emit_ctx);
    #if OSFX_CFG_AUTOLOAD_TRANSPORT
    osfx_platform_plugin_load(&ctx->platform_runtime, "transport", "");
    #endif
    #if OSFX_CFG_AUTOLOAD_TEST_PLUGIN
    osfx_platform_plugin_load(&ctx->platform_runtime, "test_plugin", "");
    #endif
    #if OSFX_CFG_AUTOLOAD_PORT_FORWARDER
    osfx_platform_plugin_load(&ctx->platform_runtime, "port_forwarder", "");
    #endif

    return OSFX_GLUE_OK;
}

int osfx_glue_process_packet(
    osfx_glue_ctx* ctx,
    const uint8_t* packet,
    size_t packet_len,
    uint64_t now_ts,
    osfx_hs_result* out_result
) {
    osfx_hs_dispatch_ctx hs_ctx;
    if (!ctx || !packet || packet_len == 0U || !out_result) {
        return OSFX_GLUE_ERR_ARG;
    }
    memset(&hs_ctx, 0, sizeof(hs_ctx));
    hs_ctx.secure_store = &ctx->secure_store;
    hs_ctx.id_allocator = &ctx->id_allocator;
    hs_ctx.now_ts = now_ts;

    return osfx_hs_classify_dispatch(&hs_ctx, packet, packet_len, out_result) ? OSFX_GLUE_OK : OSFX_GLUE_ERR_RUNTIME;
}

int osfx_glue_encode_sensor_auto(
    osfx_glue_ctx* ctx,
    uint8_t tid,
    uint64_t timestamp_raw,
    const char* sensor_id,
    double input_value,
    const char* input_unit,
    uint8_t* out_packet,
    size_t out_packet_cap,
    int* out_packet_len,
    uint8_t* out_cmd
) {
    if (!ctx) {
        return OSFX_GLUE_ERR_ARG;
    }
    return osfx_core_encode_sensor_packet_auto(
               &ctx->tx_state,
               ctx->local_aid,
               tid,
               timestamp_raw,
               sensor_id,
               input_value,
               input_unit,
               out_packet,
               out_packet_cap,
               out_packet_len,
               out_cmd
           )
               ? OSFX_GLUE_OK
               : OSFX_GLUE_ERR_CODEC;
}

int osfx_glue_decode_sensor_auto(
    osfx_glue_ctx* ctx,
    const uint8_t* packet,
    size_t packet_len,
    char* out_sensor_id,
    size_t out_sensor_id_cap,
    double* out_value,
    char* out_unit,
    size_t out_unit_cap,
    osfx_packet_meta* out_meta
) {
    if (!ctx) {
        return OSFX_GLUE_ERR_ARG;
    }
    return osfx_core_decode_sensor_packet_auto(
               &ctx->rx_state,
               packet,
               packet_len,
               out_sensor_id,
               out_sensor_id_cap,
               out_value,
               out_unit,
               out_unit_cap,
               out_meta
           )
               ? OSFX_GLUE_OK
               : OSFX_GLUE_ERR_CODEC;
}

int osfx_glue_plugin_cmd(
    osfx_glue_ctx* ctx,
    const char* plugin_name,
    const char* cmd,
    const char* args,
    char* out,
    size_t out_cap
) {
    if (!ctx || !plugin_name || !cmd || !out || out_cap == 0U) {
        return OSFX_GLUE_ERR_ARG;
    }
    return osfx_platform_plugin_cmd(&ctx->platform_runtime, plugin_name, cmd, args ? args : "", out, out_cap)
               ? OSFX_GLUE_OK
               : OSFX_GLUE_ERR_RUNTIME;
}

