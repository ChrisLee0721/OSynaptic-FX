# 15 Release Playbook

## Goal

Produce a release package with validated quality gate, benchmark evidence, and complete docs.

## Step 1: Build + Tests

```powershell
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\build.ps1 -Compiler auto
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\test.ps1 -Matrix
```

## Step 2: Benchmark Gate

```powershell
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\bench.ps1 -Compiler auto -MemoryLimitKB 16
```

## Step 3: Verify Reports

- `osfx-c99/build/quality_gate_report.md`
- `osfx-c99/build/bench/bench_report.md`
- `osfx-c99/build/bench/bench_report.csv`

## Step 4: Update Release Docs

- `docs/08-release-notes.md`
- `docs/09-mirror-coverage-report.md`
- `docs/13-performance-summary.md`
- `docs/CHANGELOG.md`

## Step 5: Final Acceptance

Use checklist in `docs/10-acceptance-checklist.md` and mark all items complete.

## Deliverable Set

- Static library output (`libosfx_core.a` / `osfx_core.lib`)
- Quality gate report
- Benchmark reports
- Release notes + mirror coverage + changelog

