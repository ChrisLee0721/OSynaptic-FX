# osfx-c99 release benchmark report

| Sensors | JSON bytes | Packet bytes | Compression (vs JSON) | Reduction | P50 packet us | P95 packet us | P50 per-sensor us | P95 per-sensor us | Avg per-sensor us | Sensors/s |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 98 | 29 | 29.59% | 70.41% | 0.600 | 0.700 | 0.600 | 0.700 | 0.666 | 1501773 |
| 4 | 264 | 110 | 41.67% | 58.33% | 1.700 | 1.800 | 0.425 | 0.450 | 0.446 | 2243708 |
| 8 | 486 | 220 | 45.27% | 54.73% | 3.400 | 3.500 | 0.425 | 0.438 | 0.432 | 2313790 |
| 16 | 930 | 440 | 47.31% | 52.69% | 6.800 | 7.000 | 0.425 | 0.438 | 0.435 | 2298422 |

- RAM working set before: 4148 KB
- RAM working set after: 4160 KB
- RAM working set delta: 12 KB
- RAM memory lock limit: 16 KB
- RAM memory lock status: PASS
- Struct bytes: platform_runtime=26776, protocol_matrix=912, fusion_state=12288
