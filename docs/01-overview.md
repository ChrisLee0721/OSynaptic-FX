# 01 Overview

## Project Positioning

`osfx-c99` is a C99 embedded implementation of the OpenSynaptic protocol core, targeting to provide statically linkable, portable, and verifiable core capabilities, without pursuing 1:1 replication of the full platform service ecosystem.

## Implemented Scope

- Codecs and Checksums: Base62, CRC8, CRC16/CCITT.
- Packet Handling: FULL/DIFF/HEART encoding and minimal metadata decoding.
- Fusion State Machine: Automatic policy switching and playback path.
- Security Layer: Session state, key derivation, timestamp monotonicity protection, persistence.
- Runtime: Transporter runtime, protocol matrix, service runtime.
- Plugin System (current phase): `transport` lite, `test_plugin` lite, `port_forwarder` full.
- CLI: Lightweight command routing and standalone entry point `tools/osfx_cli_main.c`.

## Phase Status

- `P0`: Core closed-loop completion.
- `P1`: Library data-driven mirror completion (Units/Prefixes/Symbols).
- `P2`: Plugin/service runtime mirror completion.
- `P3`: Security control plane boundary handling completion.
- `P4`: ID lease policy enhancement completion.
- Current closure direction: Release documentation, mirror coverage, and continuous optimization.

## Non-Goals (Current)

- Excluding `web/sql/dependency_manager/env_guard`.
- Not implementing full OpenSynaptic service ecosystem and complex operations plane.

