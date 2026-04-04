#ifndef OSFX_LIBRARY_CATALOG_H
#define OSFX_LIBRARY_CATALOG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct osfx_unit_record {
    const char* key;
    const char* ucum;
    const char* base_unit;
    double factor;
    double offset;
    int can_take_prefix;
} osfx_unit_record;

typedef struct osfx_prefix_record {
    const char* key;
    double factor;
} osfx_prefix_record;

int osfx_library_catalog_ready(void);
size_t osfx_library_unit_count(void);
size_t osfx_library_prefix_count(void);

const osfx_unit_record* osfx_library_find_unit(const char* unit_key);
const osfx_prefix_record* osfx_library_find_prefix(const char* prefix_key);
const osfx_prefix_record* osfx_library_prefix_at(size_t index);

int osfx_library_unit_symbol(const char* unit_key, const char** out_symbol);
int osfx_library_state_symbol(const char* state_key, const char** out_symbol);

#ifdef __cplusplus
}
#endif

#endif

