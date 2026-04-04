#include "../include/osfx_service_runtime.h"

#include <string.h>

static int copy_text(char* out, size_t cap, const char* text) {
    size_t n;
    if (!out || cap == 0 || !text) {
        return 0;
    }
    n = strlen(text);
    if (n + 1 > cap) {
        return 0;
    }
    memcpy(out, text, n + 1);
    return 1;
}

static osfx_service_slot* find_slot(osfx_service_runtime* rt, const char* name) {
    size_t i;
    if (!rt || !name) {
        return NULL;
    }
    for (i = 0; i < OSFX_SERVICE_MAX; ++i) {
        if (rt->services[i].used && strcmp(rt->services[i].name, name) == 0) {
            return &rt->services[i];
        }
    }
    return NULL;
}

static const osfx_service_slot* find_slot_const(const osfx_service_runtime* rt, const char* name) {
    size_t i;
    if (!rt || !name) {
        return NULL;
    }
    for (i = 0; i < OSFX_SERVICE_MAX; ++i) {
        if (rt->services[i].used && strcmp(rt->services[i].name, name) == 0) {
            return &rt->services[i];
        }
    }
    return NULL;
}

static osfx_service_slot* alloc_slot(osfx_service_runtime* rt) {
    size_t i;
    if (!rt) {
        return NULL;
    }
    for (i = 0; i < OSFX_SERVICE_MAX; ++i) {
        if (!rt->services[i].used) {
            return &rt->services[i];
        }
    }
    return NULL;
}

void osfx_service_runtime_init(osfx_service_runtime* rt) {
    if (rt) {
        memset(rt, 0, sizeof(*rt));
    }
}

int osfx_service_register(
    osfx_service_runtime* rt,
    const char* name,
    void* instance,
    osfx_service_init_fn init_fn,
    osfx_service_tick_fn tick_fn,
    osfx_service_close_fn close_fn,
    osfx_service_reload_fn reload_fn,
    const char* initial_config
) {
    return osfx_service_register_ex(
        rt,
        name,
        instance,
        init_fn,
        tick_fn,
        close_fn,
        reload_fn,
        NULL,
        initial_config
    );
}

int osfx_service_register_ex(
    osfx_service_runtime* rt,
    const char* name,
    void* instance,
    osfx_service_init_fn init_fn,
    osfx_service_tick_fn tick_fn,
    osfx_service_close_fn close_fn,
    osfx_service_reload_fn reload_fn,
    osfx_service_cmd_fn cmd_fn,
    const char* initial_config
) {
    osfx_service_slot* s;
    if (!rt || !name || !init_fn) {
        return 0;
    }
    s = find_slot(rt, name);
    if (!s) {
        s = alloc_slot(rt);
    }
    if (!s) {
        return 0;
    }

    if (!s->used) {
        memset(s, 0, sizeof(*s));
        s->used = 1;
        if (!copy_text(s->name, sizeof(s->name), name)) {
            memset(s, 0, sizeof(*s));
            return 0;
        }
    }

    s->instance = instance;
    s->init_fn = init_fn;
    s->tick_fn = tick_fn;
    s->close_fn = close_fn;
    s->reload_fn = reload_fn;
    s->cmd_fn = cmd_fn;
    s->enabled = 1;
    if (initial_config) {
        if (!copy_text(s->config, sizeof(s->config), initial_config)) {
            return 0;
        }
    }
    return 1;
}

int osfx_service_set_enabled(osfx_service_runtime* rt, const char* name, int enabled) {
    osfx_service_slot* s = find_slot(rt, name);
    if (!s) {
        return 0;
    }
    s->enabled = enabled ? 1 : 0;
    return 1;
}

int osfx_service_set_config(osfx_service_runtime* rt, const char* name, const char* config) {
    osfx_service_slot* s = find_slot(rt, name);
    if (!s || !config) {
        return 0;
    }
    return copy_text(s->config, sizeof(s->config), config);
}

const char* osfx_service_get_config(const osfx_service_runtime* rt, const char* name) {
    const osfx_service_slot* s = find_slot_const(rt, name);
    if (!s) {
        return NULL;
    }
    return s->config;
}

int osfx_service_init_all(osfx_service_runtime* rt, const char* default_config) {
    size_t i;
    int ok = 1;
    if (!rt) {
        return 0;
    }
    for (i = 0; i < OSFX_SERVICE_MAX; ++i) {
        osfx_service_slot* s = &rt->services[i];
        const char* cfg;
        int rc;
        if (!s->used || !s->enabled || !s->init_fn) {
            continue;
        }
        cfg = (s->config[0] != '\0') ? s->config : (default_config ? default_config : "");
        rc = s->init_fn(s->instance, cfg);
        s->last_rc = rc;
        s->init_count++;
        s->running = (rc ? 1 : 0);
        if (!rc) {
            ok = 0;
        }
    }
    return ok;
}

int osfx_service_tick_all(osfx_service_runtime* rt, uint64_t now_ts) {
    size_t i;
    int ok = 1;
    if (!rt) {
        return 0;
    }
    for (i = 0; i < OSFX_SERVICE_MAX; ++i) {
        osfx_service_slot* s = &rt->services[i];
        int rc;
        if (!s->used || !s->enabled || !s->running || !s->tick_fn) {
            continue;
        }
        rc = s->tick_fn(s->instance, now_ts);
        s->last_rc = rc;
        s->tick_count++;
        if (!rc) {
            ok = 0;
        }
    }
    return ok;
}

int osfx_service_reload(osfx_service_runtime* rt, const char* name, const char* config) {
    osfx_service_slot* s = find_slot(rt, name);
    int rc;
    if (!s || !s->enabled) {
        return 0;
    }
    if (config && !copy_text(s->config, sizeof(s->config), config)) {
        return 0;
    }
    if (s->reload_fn) {
        rc = s->reload_fn(s->instance, s->config);
        s->last_rc = rc;
        s->reload_count++;
        if (!rc) {
            s->running = 0;
        }
        return rc ? 1 : 0;
    }

    if (s->running && s->close_fn) {
        s->close_fn(s->instance);
        s->close_count++;
        s->running = 0;
    }
    if (!s->init_fn) {
        return 0;
    }
    rc = s->init_fn(s->instance, s->config);
    s->last_rc = rc;
    s->init_count++;
    s->reload_count++;
    s->running = rc ? 1 : 0;
    return rc ? 1 : 0;
}

int osfx_service_close_all(osfx_service_runtime* rt) {
    size_t i;
    int ok = 1;
    if (!rt) {
        return 0;
    }
    for (i = 0; i < OSFX_SERVICE_MAX; ++i) {
        osfx_service_slot* s = &rt->services[i];
        int rc = 1;
        if (!s->used || !s->running) {
            continue;
        }
        if (s->close_fn) {
            rc = s->close_fn(s->instance);
            s->last_rc = rc;
            s->close_count++;
            if (!rc) {
                ok = 0;
            }
        }
        s->running = 0;
    }
    return ok;
}

int osfx_service_get_status(const osfx_service_runtime* rt, const char* name, osfx_service_status* out_status) {
    const osfx_service_slot* s = find_slot_const(rt, name);
    if (!s || !out_status) {
        return 0;
    }
    out_status->enabled = s->enabled;
    out_status->running = s->running;
    out_status->last_rc = s->last_rc;
    out_status->init_count = s->init_count;
    out_status->tick_count = s->tick_count;
    out_status->close_count = s->close_count;
    out_status->reload_count = s->reload_count;
    out_status->cmd_count = s->cmd_count;
    return 1;
}

size_t osfx_service_count(const osfx_service_runtime* rt) {
    size_t i;
    size_t n = 0;
    if (!rt) {
        return 0;
    }
    for (i = 0; i < OSFX_SERVICE_MAX; ++i) {
        if (rt->services[i].used) {
            n++;
        }
    }
    return n;
}

int osfx_service_name_at(const osfx_service_runtime* rt, size_t index, char* out_name, size_t out_name_cap) {
    size_t i;
    size_t seen = 0;
    if (!rt || !out_name || out_name_cap == 0U) {
        return 0;
    }
    for (i = 0; i < OSFX_SERVICE_MAX; ++i) {
        if (!rt->services[i].used) {
            continue;
        }
        if (seen == index) {
            return copy_text(out_name, out_name_cap, rt->services[i].name);
        }
        seen++;
    }
    return 0;
}

int osfx_service_load(osfx_service_runtime* rt, const char* name, const char* default_config) {
    osfx_service_slot* s = find_slot(rt, name);
    const char* cfg;
    int rc;
    if (!s || !s->enabled || !s->init_fn) {
        return 0;
    }
    cfg = (s->config[0] != '\0') ? s->config : (default_config ? default_config : "");
    rc = s->init_fn(s->instance, cfg);
    s->last_rc = rc;
    s->init_count++;
    s->running = rc ? 1 : 0;
    return rc ? 1 : 0;
}

int osfx_service_command(osfx_service_runtime* rt, const char* name, const char* cmd, const char* args, char* out, size_t out_cap) {
    osfx_service_slot* s = find_slot(rt, name);
    int rc;
    if (!s || !s->enabled || !s->running || !s->cmd_fn || !cmd) {
        return 0;
    }
    rc = s->cmd_fn(s->instance, cmd, args, out, out_cap);
    s->last_rc = rc;
    s->cmd_count++;
    return rc ? 1 : 0;
}

