#include "../include/osfx_plugin_port_forwarder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int copy_text(char* out, size_t cap, const char* src) {
    size_t n;
    if (!out || cap == 0U || !src) {
        return 0;
    }
    n = strlen(src);
    if (n + 1U > cap) {
        return 0;
    }
    memcpy(out, src, n + 1U);
    return 1;
}

static osfx_pf_rule* find_rule(osfx_plugin_port_forwarder* plugin, const char* name) {
    size_t i;
    if (!plugin || !name) {
        return NULL;
    }
    for (i = 0; i < OSFX_PF_RULE_MAX; ++i) {
        if (plugin->rules[i].used && strcmp(plugin->rules[i].name, name) == 0) {
            return &plugin->rules[i];
        }
    }
    return NULL;
}

static osfx_pf_rule* alloc_rule(osfx_plugin_port_forwarder* plugin) {
    size_t i;
    if (!plugin) {
        return NULL;
    }
    for (i = 0; i < OSFX_PF_RULE_MAX; ++i) {
        if (!plugin->rules[i].used) {
            return &plugin->rules[i];
        }
    }
    return NULL;
}

static int parse_port_token(const char* token, uint16_t* out_port, int* out_has_port) {
    long v;
    if (!token || !out_port || !out_has_port) {
        return 0;
    }
    if (strcmp(token, "*") == 0) {
        *out_has_port = 0;
        *out_port = 0U;
        return 1;
    }
    v = strtol(token, NULL, 10);
    if (v <= 0 || v > 65535L) {
        return 0;
    }
    *out_has_port = 1;
    *out_port = (uint16_t)v;
    return 1;
}

static int parse_config_path(const char* config, char* out_path, size_t out_cap) {
    const char* p;
    if (!config || !out_path || out_cap == 0U) {
        return 0;
    }
    p = strstr(config, "path=");
    if (!p) {
        return 0;
    }
    p += 5;
    return copy_text(out_path, out_cap, p);
}

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

void osfx_plugin_port_forwarder_init_ctx(osfx_plugin_port_forwarder* plugin, osfx_pf_emit_fn emit_fn, void* emit_ctx) {
    if (!plugin) {
        return;
    }
    memset(plugin, 0, sizeof(*plugin));
    plugin->emit_fn = emit_fn;
    plugin->emit_ctx = emit_ctx;
}

int osfx_plugin_port_forwarder_init(void* instance, const char* config) {
    osfx_plugin_port_forwarder* p = (osfx_plugin_port_forwarder*)instance;
    if (!p) {
        return 0;
    }
    p->initialized = 1;
    if (config) {
        parse_config_path(config, p->persist_path, sizeof(p->persist_path));
        if (p->persist_path[0] != '\0') {
            osfx_pf_load(p, p->persist_path);
        }
    }
    return 1;
}

int osfx_plugin_port_forwarder_tick(void* instance, uint64_t now_ts) {
    osfx_plugin_port_forwarder* p = (osfx_plugin_port_forwarder*)instance;
    (void)now_ts;
    return (p && p->initialized) ? 1 : 0;
}

int osfx_plugin_port_forwarder_close(void* instance) {
    osfx_plugin_port_forwarder* p = (osfx_plugin_port_forwarder*)instance;
    if (!p) {
        return 0;
    }
    if (p->persist_path[0] != '\0') {
        osfx_pf_save(p, p->persist_path);
    }
    p->initialized = 0;
    return 1;
}

int osfx_plugin_port_forwarder_reload(void* instance, const char* config) {
    return osfx_plugin_port_forwarder_init(instance, config);
}

int osfx_pf_add_rule(
    osfx_plugin_port_forwarder* plugin,
    const char* name,
    const char* from_proto,
    const char* from_port_token,
    const char* to_proto,
    uint16_t to_port
) {
    osfx_pf_rule* r;
    uint16_t port = 0;
    int has_port = 0;
    if (!plugin || !name || !from_proto || !from_port_token || !to_proto || to_port == 0U) {
        return 0;
    }
    if (!parse_port_token(from_port_token, &port, &has_port)) {
        return 0;
    }
    r = find_rule(plugin, name);
    if (!r) {
        r = alloc_rule(plugin);
    }
    if (!r) {
        return 0;
    }
    memset(r, 0, sizeof(*r));
    r->used = 1;
    r->enabled = 1;
    r->has_from_port = has_port;
    r->from_port = port;
    r->to_port = to_port;
    if (!copy_text(r->name, sizeof(r->name), name)) {
        return 0;
    }
    if (!copy_text(r->from_proto, sizeof(r->from_proto), from_proto)) {
        return 0;
    }
    if (!copy_text(r->to_proto, sizeof(r->to_proto), to_proto)) {
        return 0;
    }
    return 1;
}

int osfx_pf_remove_rule(osfx_plugin_port_forwarder* plugin, const char* name) {
    osfx_pf_rule* r = find_rule(plugin, name);
    if (!r) {
        return 0;
    }
    memset(r, 0, sizeof(*r));
    return 1;
}

int osfx_pf_set_rule_enabled(osfx_plugin_port_forwarder* plugin, const char* name, int enabled) {
    osfx_pf_rule* r = find_rule(plugin, name);
    if (!r) {
        return 0;
    }
    r->enabled = enabled ? 1 : 0;
    return 1;
}

int osfx_pf_forward(
    osfx_plugin_port_forwarder* plugin,
    const char* from_proto,
    uint16_t from_port,
    const uint8_t* payload,
    size_t payload_len,
    char* out_route,
    size_t out_route_cap
) {
    size_t i;
    if (!plugin || !from_proto || !payload || !plugin->emit_fn) {
        return 0;
    }
    for (i = 0; i < OSFX_PF_RULE_MAX; ++i) {
        osfx_pf_rule* r = &plugin->rules[i];
        if (!r->used || !r->enabled) {
            continue;
        }
        if (strcmp(r->from_proto, from_proto) != 0) {
            continue;
        }
        if (r->has_from_port && r->from_port != from_port) {
            continue;
        }
        if (!plugin->emit_fn(r->to_proto, r->to_port, payload, payload_len, plugin->emit_ctx)) {
            plugin->total_dropped++;
            return 0;
        }
        r->hit_count++;
        plugin->total_forwarded++;
        if (out_route && out_route_cap > 0U) {
            snprintf(out_route, out_route_cap, "%s:%u", r->to_proto, (unsigned)r->to_port);
        }
        return 1;
    }
    plugin->total_dropped++;
    return 0;
}

int osfx_pf_save(const osfx_plugin_port_forwarder* plugin, const char* path) {
    FILE* fp;
    size_t i;
    if (!plugin || !path) {
        return 0;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    for (i = 0; i < OSFX_PF_RULE_MAX; ++i) {
        const osfx_pf_rule* r = &plugin->rules[i];
        if (!r->used) {
            continue;
        }
        fprintf(
            fp,
            "%s,%d,%s,%d,%u,%s,%u,%llu\n",
            r->name,
            r->enabled,
            r->from_proto,
            r->has_from_port,
            (unsigned)r->from_port,
            r->to_proto,
            (unsigned)r->to_port,
            (unsigned long long)r->hit_count
        );
    }
    fclose(fp);
    return 1;
}

int osfx_pf_load(osfx_plugin_port_forwarder* plugin, const char* path) {
    FILE* fp;
    char line[256];
    if (!plugin || !path) {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    memset(plugin->rules, 0, sizeof(plugin->rules));
    while (fgets(line, sizeof(line), fp)) {
        char name[OSFX_PF_NAME_MAX];
        int enabled = 0;
        char from_proto[OSFX_PF_PROTO_MAX];
        int has_from_port = 0;
        unsigned from_port = 0U;
        char to_proto[OSFX_PF_PROTO_MAX];
        unsigned to_port = 0U;
        unsigned long long hit_count = 0ULL;
        osfx_pf_rule* r;
        int got = sscanf(
            line,
            "%31[^,],%d,%15[^,],%d,%u,%15[^,],%u,%llu",
            name,
            &enabled,
            from_proto,
            &has_from_port,
            &from_port,
            to_proto,
            &to_port,
            &hit_count
        );
        if (got != 8) {
            continue;
        }
        r = alloc_rule(plugin);
        if (!r) {
            continue;
        }
        memset(r, 0, sizeof(*r));
        r->used = 1;
        r->enabled = enabled ? 1 : 0;
        r->has_from_port = has_from_port ? 1 : 0;
        r->from_port = (uint16_t)from_port;
        r->to_port = (uint16_t)to_port;
        r->hit_count = (uint64_t)hit_count;
        copy_text(r->name, sizeof(r->name), name);
        copy_text(r->from_proto, sizeof(r->from_proto), from_proto);
        copy_text(r->to_proto, sizeof(r->to_proto), to_proto);
    }
    fclose(fp);
    return 1;
}

int osfx_plugin_port_forwarder_command(void* instance, const char* cmd, const char* args, char* out, size_t out_cap) {
    osfx_plugin_port_forwarder* p = (osfx_plugin_port_forwarder*)instance;
    if (!p || !cmd || !out || out_cap == 0U) {
        return 0;
    }
    if (strcmp(cmd, "status") == 0) {
        snprintf(
            out,
            out_cap,
            "port_forwarder initialized=%d forwarded=%llu dropped=%llu",
            p->initialized,
            (unsigned long long)p->total_forwarded,
            (unsigned long long)p->total_dropped
        );
        return 1;
    }
    if (strcmp(cmd, "stats") == 0) {
        snprintf(out, out_cap, "forwarded=%llu dropped=%llu", (unsigned long long)p->total_forwarded, (unsigned long long)p->total_dropped);
        return 1;
    }
    if (strcmp(cmd, "list") == 0) {
        size_t i;
        size_t off = 0;
        int n = snprintf(out, out_cap, "rules=");
        if (n < 0 || (size_t)n >= out_cap) {
            return 0;
        }
        off = (size_t)n;
        for (i = 0; i < OSFX_PF_RULE_MAX; ++i) {
            const osfx_pf_rule* r = &p->rules[i];
            if (!r->used) {
                continue;
            }
            n = snprintf(
                out + off,
                out_cap - off,
                "%s%s(%s:%s->%s:%u,hits=%llu,en=%d)",
                (off > 6U) ? ";" : "",
                r->name,
                r->from_proto,
                r->has_from_port ? "port" : "*",
                r->to_proto,
                (unsigned)r->to_port,
                (unsigned long long)r->hit_count,
                r->enabled
            );
            if (n < 0 || (size_t)n >= (out_cap - off)) {
                return 0;
            }
            off += (size_t)n;
        }
        return 1;
    }
    if (strcmp(cmd, "add-rule") == 0) {
        char name[OSFX_PF_NAME_MAX];
        char from_proto[OSFX_PF_PROTO_MAX];
        char from_port_token[16];
        char to_proto[OSFX_PF_PROTO_MAX];
        unsigned to_port = 0U;
        if (!args || sscanf(args, "%31s %15s %15s %15s %u", name, from_proto, from_port_token, to_proto, &to_port) != 5) {
            snprintf(out, out_cap, "error=usage add-rule <name> <from_proto> <from_port|*> <to_proto> <to_port>");
            return 0;
        }
        if (!osfx_pf_add_rule(p, name, from_proto, from_port_token, to_proto, (uint16_t)to_port)) {
            snprintf(out, out_cap, "error=add_rule_failed");
            return 0;
        }
        snprintf(out, out_cap, "ok=1 added=%s", name);
        return 1;
    }
    if (strcmp(cmd, "remove-rule") == 0) {
        char name[OSFX_PF_NAME_MAX];
        if (!args || sscanf(args, "%31s", name) != 1) {
            snprintf(out, out_cap, "error=usage remove-rule <name>");
            return 0;
        }
        if (!osfx_pf_remove_rule(p, name)) {
            snprintf(out, out_cap, "error=remove_rule_failed");
            return 0;
        }
        snprintf(out, out_cap, "ok=1 removed=%s", name);
        return 1;
    }
    if (strcmp(cmd, "enable-rule") == 0) {
        char name[OSFX_PF_NAME_MAX];
        int enabled = 0;
        if (!args || sscanf(args, "%31s %d", name, &enabled) != 2) {
            snprintf(out, out_cap, "error=usage enable-rule <name> <0|1>");
            return 0;
        }
        if (!osfx_pf_set_rule_enabled(p, name, enabled)) {
            snprintf(out, out_cap, "error=enable_rule_failed");
            return 0;
        }
        snprintf(out, out_cap, "ok=1 rule=%s enabled=%d", name, enabled ? 1 : 0);
        return 1;
    }
    if (strcmp(cmd, "forward") == 0) {
        char from_proto[OSFX_PF_PROTO_MAX];
        unsigned from_port = 0U;
        char hex[1024];
        uint8_t payload[512];
        size_t payload_len = 0;
        char route[64];
        if (!args || sscanf(args, "%15s %u %1023s", from_proto, &from_port, hex) != 3) {
            snprintf(out, out_cap, "error=usage forward <from_proto> <from_port> <hex_payload>");
            return 0;
        }
        if (!hex_to_bytes(hex, payload, sizeof(payload), &payload_len)) {
            snprintf(out, out_cap, "error=invalid_hex_payload");
            return 0;
        }
        if (!osfx_pf_forward(p, from_proto, (uint16_t)from_port, payload, payload_len, route, sizeof(route))) {
            snprintf(out, out_cap, "error=no_match_or_emit_failed");
            return 0;
        }
        snprintf(out, out_cap, "ok=1 route=%s bytes=%llu", route, (unsigned long long)payload_len);
        return 1;
    }
    if (strcmp(cmd, "save") == 0) {
        const char* path = (args && args[0] != '\0') ? args : p->persist_path;
        if (!path || path[0] == '\0' || !osfx_pf_save(p, path)) {
            snprintf(out, out_cap, "error=save_failed");
            return 0;
        }
        snprintf(out, out_cap, "ok=1 saved=%s", path);
        return 1;
    }
    if (strcmp(cmd, "load") == 0) {
        const char* path = (args && args[0] != '\0') ? args : p->persist_path;
        if (!path || path[0] == '\0' || !osfx_pf_load(p, path)) {
            snprintf(out, out_cap, "error=load_failed");
            return 0;
        }
        snprintf(out, out_cap, "ok=1 loaded=%s", path);
        return 1;
    }
    snprintf(out, out_cap, "error=unknown_cmd");
    return 0;
}

