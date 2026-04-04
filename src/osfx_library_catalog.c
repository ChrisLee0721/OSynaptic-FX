#include "../include/osfx_library_catalog.h"

#include <ctype.h>
#include <string.h>

typedef struct osfx_symbol_pair {
    const char* key;
    const char* value;
} osfx_symbol_pair;

#include "osfx_library_data.generated.h"

static int equals_ignore_case(const char* a, const char* b) {
    unsigned char ca;
    unsigned char cb;
    if (!a || !b) {
        return 0;
    }
    while (*a && *b) {
        ca = (unsigned char)*a;
        cb = (unsigned char)*b;
        if ((unsigned char)tolower(ca) != (unsigned char)tolower(cb)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return (*a == '\0' && *b == '\0') ? 1 : 0;
}

int osfx_library_catalog_ready(void) {
    return (OSFX_UNIT_COUNT > 0U && OSFX_PREFIX_COUNT > 0U) ? 1 : 0;
}

size_t osfx_library_unit_count(void) {
    return OSFX_UNIT_COUNT;
}

size_t osfx_library_prefix_count(void) {
    return OSFX_PREFIX_COUNT;
}

const osfx_unit_record* osfx_library_find_unit(const char* unit_key) {
    size_t i;
    if (!unit_key) {
        return NULL;
    }
    for (i = 0; i < OSFX_UNIT_COUNT; ++i) {
        if (equals_ignore_case(unit_key, OSFX_UNIT_RECORDS[i].key) || equals_ignore_case(unit_key, OSFX_UNIT_RECORDS[i].ucum)) {
            return &OSFX_UNIT_RECORDS[i];
        }
    }
    return NULL;
}

const osfx_prefix_record* osfx_library_find_prefix(const char* prefix_key) {
    size_t i;
    if (!prefix_key) {
        return NULL;
    }
    for (i = 0; i < OSFX_PREFIX_COUNT; ++i) {
        if (strcmp(prefix_key, OSFX_PREFIX_RECORDS[i].key) == 0) {
            return &OSFX_PREFIX_RECORDS[i];
        }
    }
    return NULL;
}

const osfx_prefix_record* osfx_library_prefix_at(size_t index) {
    if (index >= OSFX_PREFIX_COUNT) {
        return NULL;
    }
    return &OSFX_PREFIX_RECORDS[index];
}

int osfx_library_unit_symbol(const char* unit_key, const char** out_symbol) {
    size_t i;
    if (!unit_key || !out_symbol) {
        return 0;
    }
    for (i = 0; i < OSFX_UNIT_SYMBOL_COUNT; ++i) {
        if (equals_ignore_case(unit_key, OSFX_UNIT_SYMBOLS[i].key)) {
            *out_symbol = OSFX_UNIT_SYMBOLS[i].value;
            return 1;
        }
    }
    return 0;
}

int osfx_library_state_symbol(const char* state_key, const char** out_symbol) {
    size_t i;
    if (!state_key || !out_symbol) {
        return 0;
    }
    for (i = 0; i < OSFX_STATE_SYMBOL_COUNT; ++i) {
        if (equals_ignore_case(state_key, OSFX_STATE_SYMBOLS[i].key)) {
            *out_symbol = OSFX_STATE_SYMBOLS[i].value;
            return 1;
        }
    }
    return 0;
}

