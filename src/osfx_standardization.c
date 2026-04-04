#include "../include/osfx_standardization.h"
#include "../include/osfx_library_catalog.h"

#include <string.h>

static int copy_unit(char* out_unit, size_t out_unit_cap, const char* text) {
    size_t n;
    if (!out_unit || out_unit_cap == 0 || !text) {
        return 0;
    }
    n = strlen(text);
    if (n + 1 > out_unit_cap) {
        return 0;
    }
    memcpy(out_unit, text, n + 1);
    return 1;
}

static int starts_with(const char* text, const char* prefix) {
    while (*prefix) {
        if (*text != *prefix) {
            return 0;
        }
        ++text;
        ++prefix;
    }
    return 1;
}

static int resolve_with_prefix(const char* input_unit, double* out_factor, double* out_offset, const char** out_base_unit) {
    size_t i;
    size_t n;
    char rest[64];
    if (!input_unit || !out_factor || !out_offset || !out_base_unit) {
        return 0;
    }
    n = strlen(input_unit);
    for (i = 0; i < osfx_library_prefix_count(); ++i) {
        const osfx_prefix_record* p = osfx_library_prefix_at(i);
        if (!p) {
            continue;
        }
        size_t plen = strlen(p->key);
        const osfx_unit_record* u;
        if (plen == 0 || plen >= n || !starts_with(input_unit, p->key)) {
            continue;
        }
        if (n - plen >= sizeof(rest)) {
            continue;
        }
        memcpy(rest, input_unit + plen, n - plen);
        rest[n - plen] = '\0';
        u = osfx_library_find_unit(rest);
        if (!u || !u->can_take_prefix) {
            continue;
        }
        *out_factor = u->factor * p->factor;
        *out_offset = u->offset;
        *out_base_unit = u->base_unit;
        return 1;
    }
    return 0;
}

int osfx_standardize_value(
    double input_value,
    const char* input_unit,
    double* out_value,
    char* out_unit,
    size_t out_unit_cap
) {
    const osfx_unit_record* u;
    double factor;
    double offset;
    const char* base_unit;

    if (!out_value || !input_unit) {
        return 0;
    }

    u = osfx_library_find_unit(input_unit);
    if (u) {
        *out_value = input_value * u->factor + u->offset;
        return copy_unit(out_unit, out_unit_cap, u->base_unit);
    }

    if (resolve_with_prefix(input_unit, &factor, &offset, &base_unit)) {
        *out_value = input_value * factor + offset;
        return copy_unit(out_unit, out_unit_cap, base_unit);
    }

    *out_value = input_value;
    return copy_unit(out_unit, out_unit_cap, input_unit);
}

