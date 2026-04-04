#include "../include/osfx_transporter_runtime.h"

#include <string.h>

static int copy_name(char* out, size_t cap, const char* name) {
    size_t n;
    if (!out || !name || cap == 0) {
        return 0;
    }
    n = strlen(name);
    if (n + 1 > cap) {
        return 0;
    }
    memcpy(out, name, n + 1);
    return 1;
}

void osfx_transporter_runtime_init(osfx_transporter_runtime* rt) {
    if (rt) {
        memset(rt, 0, sizeof(*rt));
    }
}

int osfx_transporter_register(
    osfx_transporter_runtime* rt,
    const char* name,
    int priority,
    osfx_send_fn send_fn,
    void* user_ctx
) {
    size_t i;
    if (!rt || !name || !send_fn) {
        return 0;
    }

    for (i = 0; i < OSFX_TRANSPORTER_MAX; ++i) {
        if (rt->drivers[i].used && strcmp(rt->drivers[i].name, name) == 0) {
            rt->drivers[i].priority = priority;
            rt->drivers[i].send_fn = send_fn;
            rt->drivers[i].user_ctx = user_ctx;
            rt->drivers[i].enabled = 1;
            rt->drivers[i].max_retries = (rt->drivers[i].max_retries < 0) ? 0 : rt->drivers[i].max_retries;
            return 1;
        }
    }

    for (i = 0; i < OSFX_TRANSPORTER_MAX; ++i) {
        if (!rt->drivers[i].used) {
            rt->drivers[i].used = 1;
            rt->drivers[i].priority = priority;
            rt->drivers[i].send_fn = send_fn;
            rt->drivers[i].user_ctx = user_ctx;
            rt->drivers[i].enabled = 1;
            rt->drivers[i].max_retries = 0;
            return copy_name(rt->drivers[i].name, sizeof(rt->drivers[i].name), name);
        }
    }

    return 0;
}

int osfx_transporter_set_enabled(osfx_transporter_runtime* rt, const char* name, int enabled) {
    size_t i;
    if (!rt || !name) {
        return 0;
    }
    for (i = 0; i < OSFX_TRANSPORTER_MAX; ++i) {
        if (rt->drivers[i].used && strcmp(rt->drivers[i].name, name) == 0) {
            rt->drivers[i].enabled = enabled ? 1 : 0;
            return 1;
        }
    }
    return 0;
}

int osfx_transporter_set_retry(osfx_transporter_runtime* rt, const char* name, int max_retries) {
    size_t i;
    if (!rt || !name) {
        return 0;
    }
    if (max_retries < 0) {
        max_retries = 0;
    }
    for (i = 0; i < OSFX_TRANSPORTER_MAX; ++i) {
        if (rt->drivers[i].used && strcmp(rt->drivers[i].name, name) == 0) {
            rt->drivers[i].max_retries = max_retries;
            return 1;
        }
    }
    return 0;
}

int osfx_transporter_dispatch_named(
    osfx_transporter_runtime* rt,
    const char* name,
    const uint8_t* payload,
    size_t payload_len
) {
    size_t i;
    if (!rt || !name || !payload) {
        return 0;
    }
    for (i = 0; i < OSFX_TRANSPORTER_MAX; ++i) {
        if (rt->drivers[i].used && rt->drivers[i].enabled && strcmp(rt->drivers[i].name, name) == 0) {
            int tries = 1 + (rt->drivers[i].max_retries > 0 ? rt->drivers[i].max_retries : 0);
            int t;
            for (t = 0; t < tries; ++t) {
                if (rt->drivers[i].send_fn(payload, payload_len, rt->drivers[i].user_ctx)) {
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}

int osfx_transporter_dispatch_auto(
    osfx_transporter_runtime* rt,
    const uint8_t* payload,
    size_t payload_len,
    char* out_used_name,
    size_t out_used_name_cap
) {
    size_t i;
    int best_idx = -1;
    int best_priority = -2147483647;

    if (!rt || !payload) {
        return 0;
    }

    for (i = 0; i < OSFX_TRANSPORTER_MAX; ++i) {
        if (!rt->drivers[i].used || !rt->drivers[i].enabled || !rt->drivers[i].send_fn) {
            continue;
        }
        if (rt->drivers[i].priority > best_priority) {
            best_priority = rt->drivers[i].priority;
            best_idx = (int)i;
        }
    }

    if (best_idx < 0) {
        return 0;
    }

    {
        int tries = 1 + (rt->drivers[best_idx].max_retries > 0 ? rt->drivers[best_idx].max_retries : 0);
        int t;
        for (t = 0; t < tries; ++t) {
            if (rt->drivers[best_idx].send_fn(payload, payload_len, rt->drivers[best_idx].user_ctx)) {
                if (out_used_name && out_used_name_cap > 0) {
                    copy_name(out_used_name, out_used_name_cap, rt->drivers[best_idx].name);
                }
                return 1;
            }
        }
    }

    for (i = 0; i < OSFX_TRANSPORTER_MAX; ++i) {
        if ((int)i == best_idx || !rt->drivers[i].used || !rt->drivers[i].enabled || !rt->drivers[i].send_fn) {
            continue;
        }
        {
            int tries = 1 + (rt->drivers[i].max_retries > 0 ? rt->drivers[i].max_retries : 0);
            int t;
            for (t = 0; t < tries; ++t) {
                if (rt->drivers[i].send_fn(payload, payload_len, rt->drivers[i].user_ctx)) {
                    if (out_used_name && out_used_name_cap > 0) {
                        copy_name(out_used_name, out_used_name_cap, rt->drivers[i].name);
                    }
                    return 1;
                }
            }
        }
    }

    return 0;
}

