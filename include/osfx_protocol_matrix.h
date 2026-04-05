#ifndef OSFX_PROTOCOL_MATRIX_H
#define OSFX_PROTOCOL_MATRIX_H

#include <stddef.h>
#include <stdint.h>

#include "osfx_transporter_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*osfx_matrix_emit_fn)(const char* protocol, const uint8_t* frame, size_t frame_len, void* user_ctx);

typedef struct osfx_protocol_matrix {
    osfx_transporter_runtime runtime;
    osfx_matrix_emit_fn emit_fn;
    void* emit_ctx;
} osfx_protocol_matrix;

void osfx_protocol_matrix_init(osfx_protocol_matrix* pm, osfx_matrix_emit_fn emit_fn, void* emit_ctx);
int osfx_protocol_matrix_register_defaults(osfx_protocol_matrix* pm);

int osfx_protocol_send_udp(osfx_protocol_matrix* pm, const uint8_t* payload, size_t payload_len);
int osfx_protocol_send_tcp(osfx_protocol_matrix* pm, const uint8_t* payload, size_t payload_len);
int osfx_protocol_send_uart(osfx_protocol_matrix* pm, const uint8_t* payload, size_t payload_len);
int osfx_protocol_send_can(osfx_protocol_matrix* pm, const uint8_t* payload, size_t payload_len);

int osfx_protocol_dispatch_auto(
    osfx_protocol_matrix* pm,
    const uint8_t* payload,
    size_t payload_len,
    char* out_used_name,
    size_t out_used_name_cap
);

#ifdef __cplusplus
}
#endif

#endif

