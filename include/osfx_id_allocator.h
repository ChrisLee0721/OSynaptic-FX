#ifndef OSFX_ID_ALLOCATOR_H
#define OSFX_ID_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OSFX_ID_MAX_ENTRIES 1024

typedef struct osfx_id_lease_entry {
    uint32_t aid;
    uint64_t leased_until;
    uint64_t last_seen;
    int in_use;
} osfx_id_lease_entry;

typedef struct osfx_id_allocator {
    uint32_t start_id;
    uint32_t end_id;
    uint64_t default_lease_seconds;
    uint64_t min_lease_seconds;
    uint64_t max_lease_seconds;
    uint64_t rate_window_seconds;
    double high_rate_threshold_per_hour;
    double high_rate_min_factor;
    double pressure_high_watermark;
    double pressure_min_factor;
    double touch_extend_factor;
    int adaptive_enabled;
    uint64_t recent_window_start;
    uint32_t recent_alloc_count;
    osfx_id_lease_entry entries[OSFX_ID_MAX_ENTRIES];
} osfx_id_allocator;

void osfx_id_allocator_init(osfx_id_allocator* alloc, uint32_t start_id, uint32_t end_id, uint64_t default_lease_seconds);
void osfx_id_set_policy(
    osfx_id_allocator* alloc,
    uint64_t min_lease_seconds,
    uint64_t rate_window_seconds,
    double high_rate_threshold_per_hour,
    double high_rate_min_factor,
    int adaptive_enabled
);
void osfx_id_set_policy_ex(
    osfx_id_allocator* alloc,
    uint64_t min_lease_seconds,
    uint64_t max_lease_seconds,
    uint64_t rate_window_seconds,
    double high_rate_threshold_per_hour,
    double high_rate_min_factor,
    double pressure_high_watermark,
    double pressure_min_factor,
    double touch_extend_factor,
    int adaptive_enabled
);
void osfx_id_allocator_cleanup_expired(osfx_id_allocator* alloc, uint64_t now_ts);
int osfx_id_allocate(osfx_id_allocator* alloc, uint64_t now_ts, uint32_t* out_aid);
int osfx_id_release(osfx_id_allocator* alloc, uint32_t aid);
int osfx_id_touch(osfx_id_allocator* alloc, uint32_t aid, uint64_t now_ts);
int osfx_id_save(const osfx_id_allocator* alloc, const char* path);
int osfx_id_load(osfx_id_allocator* alloc, const char* path);

#ifdef __cplusplus
}
#endif

#endif

