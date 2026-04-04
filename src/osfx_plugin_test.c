#include "../include/osfx_plugin_test.h"

#include <stdio.h>
#include <string.h>

void osfx_plugin_test_init_ctx(osfx_plugin_test* plugin) {
    if (!plugin) {
        return;
    }
    memset(plugin, 0, sizeof(*plugin));
}

int osfx_plugin_test_init(void* instance, const char* config) {
    osfx_plugin_test* p = (osfx_plugin_test*)instance;
    (void)config;
    if (!p) {
        return 0;
    }
    p->initialized = 1;
    return 1;
}

int osfx_plugin_test_tick(void* instance, uint64_t now_ts) {
    osfx_plugin_test* p = (osfx_plugin_test*)instance;
    (void)now_ts;
    return (p && p->initialized) ? 1 : 0;
}

int osfx_plugin_test_close(void* instance) {
    osfx_plugin_test* p = (osfx_plugin_test*)instance;
    if (!p) {
        return 0;
    }
    p->initialized = 0;
    return 1;
}

int osfx_plugin_test_reload(void* instance, const char* config) {
    return osfx_plugin_test_init(instance, config);
}

int osfx_plugin_test_command(void* instance, const char* cmd, const char* args, char* out, size_t out_cap) {
    osfx_plugin_test* p = (osfx_plugin_test*)instance;
    (void)args;
    if (!p || !cmd || !out || out_cap == 0U) {
        return 0;
    }
    if (strcmp(cmd, "status") == 0) {
        snprintf(
            out,
            out_cap,
            "test_plugin initialized=%d total_runs=%llu pass_runs=%llu last_suite=%s",
            p->initialized,
            (unsigned long long)p->total_runs,
            (unsigned long long)p->pass_runs,
            p->last_suite
        );
        return 1;
    }
    if (strcmp(cmd, "run") == 0) {
        const char* suite = (args && args[0] != '\0') ? args : "component";
        p->total_runs++;
        p->pass_runs++;
        strncpy(p->last_suite, suite, sizeof(p->last_suite) - 1U);
        p->last_suite[sizeof(p->last_suite) - 1U] = '\0';
        snprintf(out, out_cap, "ok=1 suite=%s", p->last_suite);
        return 1;
    }
    snprintf(out, out_cap, "error=unknown_cmd");
    return 0;
}

