#ifndef OSFX_STANDARDIZATION_H
#define OSFX_STANDARDIZATION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int osfx_standardize_value(
    double input_value,
    const char* input_unit,
    double* out_value,
    char* out_unit,
    size_t out_unit_cap
);

#ifdef __cplusplus
}
#endif

#endif

