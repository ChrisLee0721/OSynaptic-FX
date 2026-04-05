# 02 Architecture

## Layered Structure

- `Core Facade`: `src/osfx_core_facade.c`, aggregating standardization, encoding, packing, unpacking.
- `Protocol Core`: `solidity`, `fusion_packet`, `fusion_state`, `template_grammar`.
- `Security`: `secure_session`, `payload_crypto`, `handshake_dispatch`.
- `Runtime`: `transporter_runtime`, `protocol_matrix`, `service_runtime`.
- `Platform Scoped Plugins`: `plugin_transport`, `plugin_test`, `plugin_port_forwarder`.
- `Platform Adapter`: `platform_runtime` + `cli_lite` + `tools/osfx_cli_main.c`.
- `Glue Adapter`: `osfx_glue`, combining encoding, handshake, ID, and plugin commands into a unified API.

## Key Data Flows

1. Sensor input -> Standardization -> Base62 -> Packet encode.
2. `fusion_state` determines FULL/DIFF/HEART policy.
3. Optional secure session encryption before protocol matrix.
4. `transport` plugin can trigger auto/proto dispatch by command.
5. `port_forwarder` plugin can forward by rules from source protocol/port to target protocol/port.

## Detailed Control Flow (Server Side)

1. Receive control packet `CMD=ID_REQUEST` (at least 3 bytes with seq).
2. `osfx_hs_classify_dispatch(...)` reads `id_allocator` from dispatch context.
3. Call `osfx_id_allocate(...)` to attempt AID allocation.
4. Success: Build `ID_ASSIGN` response; Failure: Build `NACK(ID_POOL_EXHAUSTED)`.
5. Caller sends `out_result.response` back to client.

## Detailed Data Flow (Device Side)

1. `osfx_glue_encode_sensor_auto(...)` reads local `local_aid` and `tx_state`.
2. Standardization + Base62 + Packet encode (FULL/DIFF/HEART auto-select).
3. Send via protocol matrix or upper-layer transmitter.
4. Incoming/downlink packets enter `osfx_glue_process_packet(...)`.
5. If data packet, perform timestamp anti-replay/out-of-order checks; if control packet, enter handshake dispatch.

## Plugin Runtime Relationships

- `osfx_platform_runtime_init(...)` registers 3 scoped plugins to `service_runtime`.
- Plugin list/load/cmd uniformly routed by `osfx_cli_lite_run(...)`.
- Plugin state exposed via `service_runtime` counters and return strings.

## Design Principles

- Fixed Scope: Ensure protocol core and delivery control first, no non-goal services.
- API Stability: Maintain unified `osfx_core.h` entry point externally.
- Verifiable: Every change must pass native/integration/CLI smoke tests.
- Orchestration Decoupling: Unified call path via `osfx_glue`, reducing upper-layer repetitive assembly logic.

