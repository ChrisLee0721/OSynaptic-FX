# OSynaptic-FX v1.0.0

First stable release. Focuses on protocol correctness, ESP32 DRAM safety, and struct-level memory optimisation, making the library production-deployable on constrained MCU targets.

## Highlights

### Binary DIFF protocol — breaking fix

- Rewrote `osfx_fusion_state.c` to emit native binary bitmask DIFF packets compatible with the OpenSynaptic Rust DLL decoder (`auto_decompose_input_inner`).
- Format: `[mask_bytes big-endian] + for each changed slot: [u8 len][bytes]`, slot order `META_0, VAL_0, META_1, VAL_1, …`
- Replaced the old plain-text split body that caused `"Malformed DIFF payload"` errors on the server.
- Added `parse_body_slots()` and `reconstruct_body()` internal helpers.
- Both FULL and DIFF paths validated against OpenSynaptic Python/Rust codec.

### ESP32 DRAM overflow fix

- Reduced `OSFX_FUSION_MAX_ENTRIES` default from 64 → 32.
- Reduced `OSFX_FUSION_MAX_VALS` default from 16 → 8.
- Reduced `OSFX_ID_MAX_ENTRIES` default from 1024 → 128.
- Eliminated `.dram0.bss` overflow of 16 568 bytes on ESP32.
- Total typical sketch DRAM: **~20 KB** (was ~90 KB before tuning).

### Struct memory optimisation

- Moved `size_t`/`int` fields (`sensor_count`, `val_count`, `used`) to `uint8_t`, saving ~16 B/entry.
- Removed `sig_base_len` field entirely; replaced with `strnlen()` at call sites.
- Optional `osfx_sensor_slot` fields (`geohash_id`, `supplementary_message`, `resource_url`) wrapped in `#if OSFX_CFG_PAYLOAD_*` guards — saves 288 B/sensor when disabled.
- `osfx_template_msg.sensor_count` changed from `size_t` to `uint8_t`.

### Compile-time tuning via `osfx_user_config.h`

- All `OSFX_FUSION_MAX_*`, `OSFX_ID_MAX_ENTRIES`, `OSFX_TMPL_MAX_SENSORS`, `OSFX_CFG_MULTI_SENSOR_BODY_CAP`, and payload feature flags are now `#ifndef`-guarded.
- `osfx_user_config.h` is the single override point — no library header edits needed.
- `osfx_build_config.h` is the include-chain root, included automatically.

### Arduino Library packaging

- Added `keywords.txt` for Arduino IDE syntax highlighting.
- Removed empty `examples/BenchmarkFusion/` directory (would break IDE example loading).
- Updated and expanded README with architecture diagram, memory table, tuning guide, example table, and Binary DIFF protocol documentation.
- `examples/README.md` updated to reflect all 10 valid examples.

## Breaking Changes from v0.2.0

| Area | Change |
|---|---|
| Binary DIFF wire format | Now emits native bitmask DIFF; server must be OpenSynaptic ≥ codec.py fix (included in this repo) |
| `osfx_fusion_entry` layout | `sig_base_len` field removed; `sensor_count`/`val_count`/`used` now `uint8_t` |
| `osfx_sensor_slot` layout | Optional fields conditional on `OSFX_CFG_PAYLOAD_*`; default struct is smaller |
| `OSFX_ID_MAX_ENTRIES` default | 1024 → 128 (override in `osfx_user_config.h` if you need more) |
| `OSFX_FUSION_MAX_ENTRIES` default | 64 → 32 |
| `OSFX_FUSION_MAX_VALS` default | 16 → 8 |

## Before Publishing to Arduino Library Manager

- [ ] Replace `REPLACE_WITH_REAL_ORG` in `library.properties` (`url=`) with the real GitHub organisation/user name.
- [ ] Replace `REPLACE_WITH_REAL_ORG` in `CONTRIBUTING.md` with the real GitHub organisation/user name.

## Build & Test

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Compiler auto
powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1 -Compiler auto
```

Arduino CLI validation:

```bash
arduino-cli compile --fqbn arduino:avr:uno    examples/EasyQuickStart
arduino-cli compile --fqbn arduino:avr:uno    examples/BasicEncode
arduino-cli compile --fqbn esp32:esp32:esp32  examples/ESP32WiFiMultiSensorAuto
arduino-cli compile --fqbn esp32:esp32:esp32  examples/QuickBench
```

## Artifact Matrix

Static libraries (`.a`) published for:

- `esp32` (Xtensa LX6)
- `atmega328p` / `avr` (AVR)
- `rp2040` (ARM Cortex-M0+)
- `cortex-m0plus` (generic Cortex-M0+)
- `stm32` (ARM Cortex-M)
- `riscv32` (rv32imac bare-metal)
