#include "../include/osfx_protocol_matrix.h"

#include <string.h>

static int matrix_emit(osfx_protocol_matrix* pm, const char* proto, const uint8_t* frame, size_t frame_len) {
    if (!pm || !pm->emit_fn) {
        return 0;
    }
    return pm->emit_fn(proto, frame, frame_len, pm->emit_ctx) ? 1 : 0;
}

int osfx_protocol_send_udp(osfx_protocol_matrix* pm, const uint8_t* payload, size_t payload_len) {
    return matrix_emit(pm, "udp", payload, payload_len);
}

int osfx_protocol_send_tcp(osfx_protocol_matrix* pm, const uint8_t* payload, size_t payload_len) {
    uint8_t frame[1500];
    if (payload_len + 2U > sizeof(frame)) {
        return 0;
    }
    frame[0] = (uint8_t)((payload_len >> 8) & 0xFFU);
    frame[1] = (uint8_t)(payload_len & 0xFFU);
    memcpy(frame + 2, payload, payload_len);
    return matrix_emit(pm, "tcp", frame, payload_len + 2U);
}

int osfx_protocol_send_uart(osfx_protocol_matrix* pm, const uint8_t* payload, size_t payload_len) {
    uint8_t frame[1500];
    if (payload_len + 2U > sizeof(frame)) {
        return 0;
    }
    frame[0] = 0x02;
    memcpy(frame + 1, payload, payload_len);
    frame[payload_len + 1] = 0x03;
    return matrix_emit(pm, "uart", frame, payload_len + 2U);
}

int osfx_protocol_send_can(osfx_protocol_matrix* pm, const uint8_t* payload, size_t payload_len) {
    size_t off = 0;
    uint8_t frame[16];
    size_t idx = 0;
    while (off < payload_len) {
        size_t chunk = (payload_len - off > 8U) ? 8U : (payload_len - off);
        frame[0] = (uint8_t)idx;
        frame[1] = (uint8_t)chunk;
        memcpy(frame + 2, payload + off, chunk);
        if (!matrix_emit(pm, "can", frame, chunk + 2U)) {
            return 0;
        }
        off += chunk;
        ++idx;
    }
    return 1;
}

static int cb_udp(const uint8_t* payload, size_t payload_len, void* user_ctx) {
    return osfx_protocol_send_udp((osfx_protocol_matrix*)user_ctx, payload, payload_len);
}
static int cb_tcp(const uint8_t* payload, size_t payload_len, void* user_ctx) {
    return osfx_protocol_send_tcp((osfx_protocol_matrix*)user_ctx, payload, payload_len);
}
static int cb_uart(const uint8_t* payload, size_t payload_len, void* user_ctx) {
    return osfx_protocol_send_uart((osfx_protocol_matrix*)user_ctx, payload, payload_len);
}
static int cb_can(const uint8_t* payload, size_t payload_len, void* user_ctx) {
    return osfx_protocol_send_can((osfx_protocol_matrix*)user_ctx, payload, payload_len);
}

void osfx_protocol_matrix_init(osfx_protocol_matrix* pm, osfx_matrix_emit_fn emit_fn, void* emit_ctx) {
    if (!pm) {
        return;
    }
    memset(pm, 0, sizeof(*pm));
    pm->emit_fn = emit_fn;
    pm->emit_ctx = emit_ctx;
    osfx_transporter_runtime_init(&pm->runtime);
}

int osfx_protocol_matrix_register_defaults(osfx_protocol_matrix* pm) {
    if (!pm) {
        return 0;
    }
    if (!osfx_transporter_register(&pm->runtime, "udp", 40, cb_udp, pm)) {
        return 0;
    }
    osfx_transporter_set_retry(&pm->runtime, "udp", 0);
    if (!osfx_transporter_register(&pm->runtime, "tcp", 35, cb_tcp, pm)) {
        return 0;
    }
    osfx_transporter_set_retry(&pm->runtime, "tcp", 1);
    if (!osfx_transporter_register(&pm->runtime, "uart", 20, cb_uart, pm)) {
        return 0;
    }
    osfx_transporter_set_retry(&pm->runtime, "uart", 1);
    if (!osfx_transporter_register(&pm->runtime, "can", 15, cb_can, pm)) {
        return 0;
    }
    osfx_transporter_set_retry(&pm->runtime, "can", 0);
    return 1;
}

int osfx_protocol_dispatch_auto(
    osfx_protocol_matrix* pm,
    const uint8_t* payload,
    size_t payload_len,
    char* out_used_name,
    size_t out_used_name_cap
) {
    if (!pm) {
        return 0;
    }
    return osfx_transporter_dispatch_auto(&pm->runtime, payload, payload_len, out_used_name, out_used_name_cap);
}

