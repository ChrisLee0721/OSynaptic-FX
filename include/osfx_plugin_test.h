#ifndef OSFX_PLUGIN_TEST_H
#define OSFX_PLUGIN_TEST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct osfx_plugin_test {
    int initialized;
    uint64_t total_runs;
    uint64_t pass_runs;
    char last_suite[32];
} osfx_plugin_test;

void osfx_plugin_test_init_ctx(osfx_plugin_test* plugin);

int osfx_plugin_test_init(void* instance, const char* config);
int osfx_plugin_test_tick(void* instance, uint64_t now_ts);
int osfx_plugin_test_close(void* instance);
int osfx_plugin_test_reload(void* instance, const char* config);
int osfx_plugin_test_command(void* instance, const char* cmd, const char* args, char* out, size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif

