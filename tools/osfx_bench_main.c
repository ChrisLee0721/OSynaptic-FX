#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <windows.h>
#include <psapi.h>

#include "osfx_core.h"
#include "osfx_build_config.h"

#define BENCH_TIER_COUNT 4
#define BENCH_MAX_SENSORS 16
#define BENCH_LOOPS 30000
#define BENCH_WARMUP 500

typedef struct bench_result {
    int sensors;
    size_t json_bytes;
    size_t packet_bytes;
    double ratio_pct_of_json;
    double reduction_pct;
    double p50_packet_us;
    double p95_packet_us;
    double p50_sensor_us;
    double p95_sensor_us;
    double avg_packet_us;
    double avg_sensor_us;
    double packets_per_sec;
    double sensors_per_sec;
} bench_result;

static int cmp_double(const void* a, const void* b) {
    const double da = *(const double*)a;
    const double db = *(const double*)b;
    return (da < db) ? -1 : (da > db) ? 1 : 0;
}

static double percentile_sorted(const double* sorted, size_t n, double q) {
    size_t lo;
    size_t hi;
    double idx;
    double frac;
    if (!sorted || n == 0U) {
        return 0.0;
    }
    if (q <= 0.0) {
        return sorted[0];
    }
    if (q >= 1.0) {
        return sorted[n - 1U];
    }
    idx = q * (double)(n - 1U);
    lo = (size_t)idx;
    hi = (lo + 1U < n) ? (lo + 1U) : lo;
    frac = idx - (double)lo;
    return sorted[lo] + (sorted[hi] - sorted[lo]) * frac;
}

static SIZE_T get_working_set_kb(void) {
    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return 0;
    }
    return (SIZE_T)(pmc.WorkingSetSize / 1024U);
}

static double qpc_us(LARGE_INTEGER freq) {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return ((double)t.QuadPart * 1000000.0) / (double)freq.QuadPart;
}

static void build_sensor_batch(osfx_core_sensor_input* sensors, int n, int step) {
    static const char* ids[BENCH_MAX_SENSORS] = {
        "S01","S02","S03","S04","S05","S06","S07","S08",
        "S09","S10","S11","S12","S13","S14","S15","S16"
    };
    int i;
    for (i = 0; i < n; ++i) {
        sensors[i].sensor_id = ids[i];
        sensors[i].sensor_state = "OK";
        sensors[i].unit = (i % 2 == 0) ? "kPa" : "cel";
        sensors[i].value = (i % 2 == 0) ? (100.0 + (double)(step % 13)) : (20.0 + (double)(step % 17));
    }
}

static size_t build_json_baseline_bytes(const osfx_core_sensor_input* sensors, int n) {
    char json[4096];
    size_t off = 0;
    int i;
    int w = snprintf(json + off, sizeof(json) - off, "{\"node\":\"N1\",\"state\":\"ONLINE\",\"sensors\":[");
    if (w <= 0) {
        return 0U;
    }
    off += (size_t)w;
    for (i = 0; i < n; ++i) {
        w = snprintf(
            json + off,
            sizeof(json) - off,
            "%s{\"id\":\"%s\",\"state\":\"%s\",\"value\":%.4f,\"unit\":\"%s\"}",
            (i == 0) ? "" : ",",
            sensors[i].sensor_id,
            sensors[i].sensor_state,
            sensors[i].value,
            sensors[i].unit
        );
        if (w <= 0 || (size_t)w >= sizeof(json) - off) {
            return 0U;
        }
        off += (size_t)w;
    }
    w = snprintf(json + off, sizeof(json) - off, "]}");
    if (w <= 0 || (size_t)w >= sizeof(json) - off) {
        return 0U;
    }
    off += (size_t)w;
    return off;
}

static int run_tier_bench(int sensors_n, LARGE_INTEGER freq, bench_result* out) {
    osfx_core_sensor_input sensors[BENCH_MAX_SENSORS];
    uint8_t packet[1024];
    int packet_len = 0;
    int i;
    int j;
    double* sample_us;
    double total_us = 0.0;
    size_t json_bytes;
    size_t packet_bytes;

    if (!out || sensors_n <= 0 || sensors_n > BENCH_MAX_SENSORS) {
        return 0;
    }

    sample_us = (double*)malloc(sizeof(double) * BENCH_LOOPS);
    if (!sample_us) {
        return 0;
    }

    build_sensor_batch(sensors, sensors_n, 0);
    json_bytes = build_json_baseline_bytes(sensors, sensors_n);
    if (json_bytes == 0U) {
        free(sample_us);
        return 0;
    }

    packet_bytes = 0U;
    for (j = 0; j < sensors_n; ++j) {
        if (!osfx_core_encode_sensor_packet(
                42U,
                1U,
                1710000000ULL + (uint64_t)j,
                sensors[j].sensor_id,
                sensors[j].value,
                sensors[j].unit,
                packet,
                sizeof(packet),
                &packet_len)) {
            free(sample_us);
            return 0;
        }
        packet_bytes += (size_t)packet_len;
    }

    for (i = 0; i < BENCH_WARMUP; ++i) {
        build_sensor_batch(sensors, sensors_n, i);
        for (j = 0; j < sensors_n; ++j) {
            if (!osfx_core_encode_sensor_packet(
                    42U,
                    1U,
                    1710000100ULL + (uint64_t)i,
                    sensors[j].sensor_id,
                    sensors[j].value,
                    sensors[j].unit,
                    packet,
                    sizeof(packet),
                    &packet_len)) {
                free(sample_us);
                return 0;
            }
        }
    }

    for (i = 0; i < BENCH_LOOPS; ++i) {
        double t0;
        double t1;
        build_sensor_batch(sensors, sensors_n, i + BENCH_WARMUP);
        t0 = qpc_us(freq);
        for (j = 0; j < sensors_n; ++j) {
            if (!osfx_core_encode_sensor_packet(
                    42U,
                    1U,
                    1710001000ULL + (uint64_t)i,
                    sensors[j].sensor_id,
                    sensors[j].value,
                    sensors[j].unit,
                    packet,
                    sizeof(packet),
                    &packet_len)) {
                free(sample_us);
                return 0;
            }
        }
        t1 = qpc_us(freq);
        sample_us[i] = t1 - t0;
        total_us += sample_us[i];
    }

    qsort(sample_us, BENCH_LOOPS, sizeof(double), cmp_double);

    out->sensors = sensors_n;
    out->json_bytes = json_bytes;
    out->packet_bytes = packet_bytes;
    out->ratio_pct_of_json = ((double)packet_bytes / (double)json_bytes) * 100.0;
    out->reduction_pct = 100.0 - out->ratio_pct_of_json;
    out->p50_packet_us = percentile_sorted(sample_us, BENCH_LOOPS, 0.50);
    out->p95_packet_us = percentile_sorted(sample_us, BENCH_LOOPS, 0.95);
    out->p50_sensor_us = out->p50_packet_us / (double)sensors_n;
    out->p95_sensor_us = out->p95_packet_us / (double)sensors_n;
    out->avg_packet_us = total_us / (double)BENCH_LOOPS;
    out->avg_sensor_us = out->avg_packet_us / (double)sensors_n;
    out->packets_per_sec = (out->avg_packet_us > 0.0) ? (1000000.0 / out->avg_packet_us) : 0.0;
    out->sensors_per_sec = out->packets_per_sec * (double)sensors_n;

    free(sample_us);
    return 1;
}

static int write_reports(
    const char* out_dir,
    const bench_result* rows,
    size_t n,
    SIZE_T ws_before,
    SIZE_T ws_after,
    long long memory_limit_kb,
    int memory_lock_enabled,
    int memory_lock_pass
) {
    char csv_path[MAX_PATH];
    char md_path[MAX_PATH];
    FILE* csv;
    FILE* md;
    size_t i;

    if (!out_dir || !rows || n == 0U) {
        return 0;
    }

    CreateDirectoryA(out_dir, NULL);
    snprintf(csv_path, sizeof(csv_path), "%s\\bench_report.csv", out_dir);
    snprintf(md_path, sizeof(md_path), "%s\\bench_report.md", out_dir);

    csv = fopen(csv_path, "wb");
    md = fopen(md_path, "wb");
    if (!csv || !md) {
        if (csv) fclose(csv);
        if (md) fclose(md);
        return 0;
    }

    fprintf(csv, "tier_sensors,json_bytes,packet_bytes,ratio_pct,reduction_pct,p50_packet_us,p95_packet_us,p50_sensor_us,p95_sensor_us,avg_packet_us,avg_sensor_us,packets_per_sec,sensors_per_sec\n");
    for (i = 0; i < n; ++i) {
        const bench_result* r = &rows[i];
        fprintf(
            csv,
            "%d,%llu,%llu,%.2f,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.0f,%.0f\n",
            r->sensors,
            (unsigned long long)r->json_bytes,
            (unsigned long long)r->packet_bytes,
            r->ratio_pct_of_json,
            r->reduction_pct,
            r->p50_packet_us,
            r->p95_packet_us,
            r->p50_sensor_us,
            r->p95_sensor_us,
            r->avg_packet_us,
            r->avg_sensor_us,
            r->packets_per_sec,
            r->sensors_per_sec
        );
    }

    fprintf(md, "# osfx-c99 release benchmark report\n\n");
    fprintf(md, "| Sensors | JSON bytes | Packet bytes | Compression (vs JSON) | Reduction | P50 packet us | P95 packet us | P50 per-sensor us | P95 per-sensor us | Avg per-sensor us | Sensors/s |\n");
    fprintf(md, "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|\n");
    for (i = 0; i < n; ++i) {
        const bench_result* r = &rows[i];
        fprintf(
            md,
            "| %d | %llu | %llu | %.2f%% | %.2f%% | %.3f | %.3f | %.3f | %.3f | %.3f | %.0f |\n",
            r->sensors,
            (unsigned long long)r->json_bytes,
            (unsigned long long)r->packet_bytes,
            r->ratio_pct_of_json,
            r->reduction_pct,
            r->p50_packet_us,
            r->p95_packet_us,
            r->p50_sensor_us,
            r->p95_sensor_us,
            r->avg_sensor_us,
            r->sensors_per_sec
        );
    }
    fprintf(md, "\n");
    fprintf(md, "- RAM working set before: %llu KB\n", (unsigned long long)ws_before);
    fprintf(md, "- RAM working set after: %llu KB\n", (unsigned long long)ws_after);
    fprintf(md, "- RAM working set delta: %lld KB\n", (long long)(ws_after - ws_before));
    if (memory_lock_enabled) {
        fprintf(md, "- RAM memory lock limit: %lld KB\n", memory_limit_kb);
        fprintf(md, "- RAM memory lock status: %s\n", memory_lock_pass ? "PASS" : "FAIL");
    } else {
        fprintf(md, "- RAM memory lock limit: disabled\n");
    }
    fprintf(md, "- Struct bytes: platform_runtime=%llu, protocol_matrix=%llu, fusion_state=%llu\n",
            (unsigned long long)sizeof(osfx_platform_runtime),
            (unsigned long long)sizeof(osfx_protocol_matrix),
            (unsigned long long)sizeof(osfx_fusion_state));

    fclose(csv);
    fclose(md);
    return 1;
}

int main(int argc, char** argv) {
    const int tiers[BENCH_TIER_COUNT] = {1, 4, 8, 16};
    bench_result rows[BENCH_TIER_COUNT];
    LARGE_INTEGER freq;
    SIZE_T ws_before;
    SIZE_T ws_after;
    osfx_platform_runtime platform;
    osfx_protocol_matrix pm;
    size_t i;
    const char* out_dir = "E:\\OSynapptic-FX\\osfx-c99\\build\\bench";
    long long memory_limit_kb = 16;
    int memory_lock_enabled = 1;
    int memory_lock_pass = 1;

    if (argc >= 2 && argv[1] && argv[1][0] != '\0') {
        out_dir = argv[1];
    }
    if (argc >= 3 && argv[2] && argv[2][0] != '\0') {
        char* endp = NULL;
        memory_limit_kb = strtoll(argv[2], &endp, 10);
        if (endp && *endp == '\0') {
            memory_lock_enabled = (memory_limit_kb > 0) ? 1 : 0;
        }
    }

    QueryPerformanceFrequency(&freq);

    for (i = 0; i < BENCH_TIER_COUNT; ++i) {
        if (!run_tier_bench(tiers[i], freq, &rows[i])) {
            printf("bench_failed=1 tier=%d\n", tiers[i]);
            return 1;
        }
    }

    ws_before = get_working_set_kb();
    osfx_protocol_matrix_init(&pm, NULL, NULL);
    osfx_platform_runtime_init(&platform, &pm, NULL, NULL);
    #if OSFX_CFG_AUTOLOAD_TRANSPORT
    osfx_platform_plugin_load(&platform, "transport", "");
    #endif
    #if OSFX_CFG_AUTOLOAD_TEST_PLUGIN
    osfx_platform_plugin_load(&platform, "test_plugin", "");
    #endif
    #if OSFX_CFG_AUTOLOAD_PORT_FORWARDER
    osfx_platform_plugin_load(&platform, "port_forwarder", "");
    #endif
    ws_after = get_working_set_kb();

    if (memory_lock_enabled) {
        long long delta = (long long)(ws_after - ws_before);
        memory_lock_pass = (delta <= memory_limit_kb) ? 1 : 0;
    }

    if (!write_reports(out_dir, rows, BENCH_TIER_COUNT, ws_before, ws_after, memory_limit_kb, memory_lock_enabled, memory_lock_pass)) {
        printf("bench_failed=1 report_write=0\n");
        return 1;
    }

    for (i = 0; i < BENCH_TIER_COUNT; ++i) {
        printf(
            "tier=%d reduction_pct=%.2f p50_sensor_us=%.3f p95_sensor_us=%.3f sensors_per_sec=%.0f\n",
            rows[i].sensors,
            rows[i].reduction_pct,
            rows[i].p50_sensor_us,
            rows[i].p95_sensor_us,
            rows[i].sensors_per_sec
        );
    }
    printf("ram_working_set_delta_kb=%lld\n", (long long)(ws_after - ws_before));
    if (memory_lock_enabled) {
        printf("ram_memory_lock_limit_kb=%lld\n", memory_limit_kb);
        printf("ram_memory_lock_status=%s\n", memory_lock_pass ? "PASS" : "FAIL");
    } else {
        printf("ram_memory_lock_limit_kb=disabled\n");
    }
    printf("bench_csv=%s\\bench_report.csv\n", out_dir);
    printf("bench_md=%s\\bench_report.md\n", out_dir);

    if (memory_lock_enabled && !memory_lock_pass) {
        printf("bench_failed=1 reason=mem_limit_exceeded\n");
        return 1;
    }

    return 0;
}

