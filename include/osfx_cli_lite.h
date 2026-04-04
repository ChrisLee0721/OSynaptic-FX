#ifndef OSFX_CLI_LITE_H
#define OSFX_CLI_LITE_H

#include <stddef.h>

#include "osfx_platform_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

int osfx_cli_lite_run(osfx_platform_runtime* rt, int argc, const char* argv[], char* out, size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif

