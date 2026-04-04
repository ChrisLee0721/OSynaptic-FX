# 12 Config Quick Reference

## Script Parameters

### `scripts/build.ps1`

- `-Compiler auto|clang|gcc|cl`
- Default: `auto`

### `scripts/test.ps1`

- `-Compiler auto|clang|gcc|cl`
- `-Matrix` (run `clang/gcc/cl` gate)
- Default: `-Compiler auto`

### `scripts/bench.ps1`

- `-Compiler auto|clang|gcc|cl`
- `-MemoryLimitKB <int>`
- Default: `-MemoryLimitKB 16`
- Set `-MemoryLimitKB 0` to disable memory lock.

## Common Config Profiles

```powershell
# Local fast validation
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\test.ps1 -Compiler auto

# Release matrix gate
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\test.ps1 -Matrix

# Release benchmark with default 16KB lock
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\bench.ps1 -Compiler auto

# Benchmark with strict lock
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\bench.ps1 -Compiler auto -MemoryLimitKB 8
```

## CLI Runtime Config Entry

- `plugin-load <name> [config]` can pass plugin-specific config strings.
- Current scoped plugins:
  - `transport`
  - `test_plugin`
  - `port_forwarder`

## Report Artifacts

- Quality gate: `osfx-c99/build/quality_gate_report.md`
- Benchmark: `osfx-c99/build/bench/bench_report.md`
- Benchmark CSV: `osfx-c99/build/bench/bench_report.csv`

