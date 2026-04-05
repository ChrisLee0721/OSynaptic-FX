#include "../include/osfx_plugin_transport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return (int)(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (int)(c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + (int)(c - 'A');
    }
    return -1;
}

static int hex_to_bytes(const char* hex, uint8_t* out, size_t out_cap, size_t* out_len) {
    size_t n;
    size_t i;
    if (!hex || !out || !out_len) {
        return 0;
    }
    n = strlen(hex);
    if ((n % 2U) != 0U || (n / 2U) > out_cap) {
        return 0;
    }
    for (i = 0; i < n / 2U; ++i) {
        int hi = hex_nibble(hex[2U * i]);
        int lo = hex_nibble(hex[2U * i + 1U]);
        if (hi < 0 || lo < 0) {
            return 0;
        }
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    *out_len = n / 2U;
    return 1;
}

void osfx_plugin_transport_init_ctx(osfx_plugin_transport* plugin, osfx_protocol_matrix* matrix) {
    if (!plugin) {
        return;
    }
    memset(plugin, 0, sizeof(*plugin));
    plugin->matrix = matrix;
}

int osfx_plugin_transport_init(void* instance, const char* config) {
    osfx_plugin_transport* p = (osfx_plugin_transport*)instance;
    (void)config;
    if (!p || !p->matrix) {
        return 0;
    }
    p->initialized = 1;
    return 1;
}

int osfx_plugin_transport_tick(void* instance, uint64_t now_ts) {
    osfx_plugin_transport* p = (osfx_plugin_transport*)instance;
    (void)now_ts;
    return (p && p->initialized) ? 1 : 0;
}

int osfx_plugin_transport_close(void* instance) {
    osfx_plugin_transport* p = (osfx_plugin_transport*)instance;
    if (!p) {
        return 0;
    }
    p->initialized = 0;
    return 1;
}

int osfx_plugin_transport_reload(void* instance, const char* config) {
    return osfx_plugin_transport_init(instance, config);
}

int osfx_plugin_transport_command(void* instance, const char* cmd, const char* args, char* out, size_t out_cap) {
    osfx_plugin_transport* p = (osfx_plugin_transport*)instance;
    if (!p || !cmd || !out || out_cap == 0U) {
        return 0;
    }
    if (strcmp(cmd, "status") == 0) {
        snprintf(
            out,
            out_cap,
            "transport initialized=%d dispatch_count=%llu last_proto=%s last_payload_len=%llu",
            p->initialized,
            (unsigned long long)p->dispatch_count,
            p->last_proto,
            (unsigned long long)p->last_payload_len
        );
        return 1;
    }
    if (strcmp(cmd, "dispatch") == 0) {
        char proto[16];
        char hex[1024];
        uint8_t payload[512];
        size_t payload_len = 0;
        int rc = 0;
        if (!args || sscanf(args, "%15s %1023s", proto, hex) != 2) {
            snprintf(out, out_cap, "error=usage dispatch <proto|auto> <hex_payload>");
            return 0;
        }
        if (!hex_to_bytes(hex, payload, sizeof(payload), &payload_len)) {
            snprintf(out, out_cap, "error=invalid_hex_payload");
            return 0;
        }
        if (strcmp(proto, "auto") == 0) {
            rc = osfx_protocol_dispatch_auto(p->matrix, payload, payload_len, p->last_proto, sizeof(p->last_proto));
        } else {
            if (strcmp(proto, "udp") == 0) {
                rc = osfx_protocol_send_udp(p->matrix, payload, payload_len);
            } else if (strcmp(proto, "tcp") == 0) {
                rc = osfx_protocol_send_tcp(p->matrix, payload, payload_len);
            } else if (strcmp(proto, "uart") == 0) {
                rc = osfx_protocol_send_uart(p->matrix, payload, payload_len);
            } else if (strcmp(proto, "can") == 0) {
                rc = osfx_protocol_send_can(p->matrix, payload, payload_len);
            }
            if (rc) {
                strncpy(p->last_proto, proto, sizeof(p->last_proto) - 1U);
                p->last_proto[sizeof(p->last_proto) - 1U] = '\0';
            }
        }
        if (!rc) {
            snprintf(out, out_cap, "error=dispatch_failed");
            return 0;
        }
        p->dispatch_count++;
        p->last_payload_len = (uint64_t)payload_len;
        snprintf(out, out_cap, "ok=1 used=%s bytes=%llu", p->last_proto, (unsigned long long)p->last_payload_len);
        return 1;
    }
    snprintf(out, out_cap, "error=unknown_cmd");
    return 0;
}

