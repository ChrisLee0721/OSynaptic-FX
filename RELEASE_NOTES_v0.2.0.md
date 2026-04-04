# OSynapptic-FX v0.2.0

Embedded-first OpenSynaptic C99 release focused on multi-platform static-library delivery, CI hardening, and safer default build behavior.

## Highlights

- Added multi-platform static library pipeline for:
  - Linux / Windows / macOS (native)
  - AVR
  - ESP32
  - ARM Cortex-M
  - STM32
  - RISC-V (rv32imac)
  - RP2040
  - Android (NDK, arm64)
  - iOS (arm64)
  - WebAssembly
- Defaulted LTO to OFF for predictable size/perf behavior across toolchains.
- Added explicit CMake option:
  - `OSFX_ENABLE_LTO` (default `OFF`)
- Added `find_package`-friendly CMake packaging/export scaffolding.
- Added/updated CI and release workflows for cross-target `.a` artifacts.
- Fixed ESP32 CI toolchain path setup in containerized builds.
- Fixed RISC-V bare-metal header/toolchain setup via picolibc.
- Added dedicated STM32 build/release jobs.

## Breaking / Behavior Notes

- LTO is now opt-in (no longer assumed in any default path).
- Build artifact directories are now expected to stay out of source control (`.gitignore` hardened).

## Build Configuration (Recommended Defaults)

```bash
cmake -S . -B build -DOSFX_ENABLE_LTO=OFF -DOSFX_ENABLE_FILE_IO=OFF -DOSFX_ENABLE_CLI=OFF
cmake --build build --config Release
```

Enable LTO only when doing platform-specific A/B validation:

```bash
cmake -S . -B build-lto -DOSFX_ENABLE_LTO=ON -DOSFX_ENABLE_FILE_IO=OFF -DOSFX_ENABLE_CLI=OFF
cmake --build build-lto --config Release
```

## Artifact Matrix

This release publishes static libraries (`.a`) for:

- `linux`
- `avr`
- `esp32`
- `cortexm`
- `stm32`
- `riscv32`
- `rp2040`
- `android`
- `ios`
- `wasm`

## Size Snapshot (Reference, local baseline)

- `OSFX_ENABLE_FILE_IO=OFF`, `OSFX_ENABLE_CLI=OFF`: about 102 KB (`libosfx_core.a`)
- `OSFX_ENABLE_FILE_IO=ON`, `OSFX_ENABLE_CLI=ON`: about 110 KB (`libosfx_core.a`)

Note: `.a` size is not equal to final firmware size.
Evaluate final firmware (`.elf/.bin`) with and without LTO per target.

## Verification Summary

- Native CMake build passes with default LTO OFF.
- CI workflow updated to produce cross-target artifacts.
- ESP32 and RISC-V CI path/toolchain issues addressed.

## Known Limitations

- Platform-level best LTO setting is target-dependent; LTO remains OFF by default.
- Final firmware size/perf should be validated on each target board before production rollout.

## Upgrade Notes

If you maintain custom build scripts:

1. Add/propagate `-DOSFX_ENABLE_LTO=OFF` explicitly (recommended baseline).
2. Keep `OSFX_ENABLE_FILE_IO` / `OSFX_ENABLE_CLI` disabled on constrained targets unless needed.
3. Re-run platform A/B if you previously forced LTO globally.

## Acknowledgements

Thanks to all maintainers and integrators driving the embedded portability and release automation work.

