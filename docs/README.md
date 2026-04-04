# osfx-c99 Documentation Library

This directory contains the complete delivery documentation library for `osfx-c99`, covering architecture, APIs, plugin scope, CLI, quality gates, release, and acceptance.

## Reading Entry Points

- Quick Navigation: `docs/SUMMARY.md`
- Engineering Overview: `docs/01-overview.md`
- Architecture Description: `docs/02-architecture.md`
- API Index: `docs/03-api-index.md`
- Plugin Scope and Commands: `docs/04-plugin-scope-and-commands.md`
- Complete Port Forwarder Documentation: `docs/05-port-forwarder.md`
- Lightweight CLI Usage: `docs/06-cli.md`
- Quality Gate and Compiler Matrix: `docs/07-quality-gate-and-compiler-matrix.md`
- Release Notes: `docs/08-release-notes.md`
- Mirror Coverage Report: `docs/09-mirror-coverage-report.md`
- Release Acceptance Checklist: `docs/10-acceptance-checklist.md`
- Benchmark Method and Calibration: `docs/11-benchmark-method.md`
- Configuration Quick Reference: `docs/12-config-quick-reference.md`
- Performance Results Summary: `docs/13-performance-summary.md`
- Troubleshooting: `docs/14-troubleshooting.md`
- Release Process: `docs/15-release-playbook.md`
- Detailed Examples Cookbook: `docs/16-examples-cookbook.md`
- Glue Code Step-by-Step Explanation: `docs/17-glue-step-by-step.md`
- Standardized Units Table: `docs/18-standardized-units.md`
- Input Specification (Send Rules and Fields): `docs/19-input-specification.md`
- Version Changelog: `docs/CHANGELOG.md`

## Current Documentation Baseline

- Plugin Policy: Only includes `transport` (lite), `test_plugin` (lite), `port_forwarder` (full).
- Explicit Exclusions: `web`, `sql`, `dependency_manager`, `env_guard`, and other service plugins.
- Quality Gate: Supports `clang/gcc/cl` matrix verification with report artifact at `osfx-c99/build/quality_gate_report.md`.
