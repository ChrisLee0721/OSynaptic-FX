# 06 CLI Lite Guide

## Entry Point

- Independent entry source code: `tools/osfx_cli_main.c`
- Routing implementation: `src/osfx_cli_lite.c`

## Command Overview

- `plugin-list`
- `plugin-load <name> [config]`
- `plugin-cmd <name> <cmd> [args...]`
- `transport-status`
- `test-plugin <subcmd> [args...]`
- `port-forwarder <subcmd> [args...]`

## Common Examples

```powershell
# List plugins
.\osfx-c99\build\osfx_cli_cl.exe plugin-list

# Load transport and view status
.\osfx-c99\build\osfx_cli_cl.exe plugin-load transport
.\osfx-c99\build\osfx_cli_cl.exe transport-status

# Run test_plugin lightweight suite
.\osfx-c99\build\osfx_cli_cl.exe test-plugin run component

# Add and trigger port forwarding rule
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder add-rule r1 udp 8080 tcp 9000
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder forward udp 8080 A1B2
```

## Multi-Compiler Executable Examples

Test scripts produce different CLI executables based on compiler:

- `clang`: `osfx-c99/build/osfx_cli_clang.exe`
- `gcc`: `osfx-c99/build/osfx_cli_gcc.exe`
- `cl`: `osfx-c99/build/osfx_cli_cl.exe`

```powershell
# clang artifact example
.\osfx-c99\build\osfx_cli_clang.exe plugin-list

# gcc artifact example
.\osfx-c99\build\osfx_cli_gcc.exe transport-status
```

## Recommended Workflow Example

```powershell
# 1) View plugins
.\osfx-c99\build\osfx_cli_cl.exe plugin-list

# 2) Load plugins
.\osfx-c99\build\osfx_cli_cl.exe plugin-load transport
.\osfx-c99\build\osfx_cli_cl.exe plugin-load test_plugin

# 3) Execute checks
.\osfx-c99\build\osfx_cli_cl.exe transport-status
.\osfx-c99\build\osfx_cli_cl.exe test-plugin run component

# 4) Add forwarding rule and trigger
.\osfx-c99\build\osfx_cli_cl.exe plugin-load port_forwarder
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder add-rule r1 udp 8080 tcp 9000
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder forward udp 8080 A1B2C3
```

## Output Conventions

- Success typically includes `ok=1` or status summary.
- Failure returns `error=...` and exits with non-zero code.

Examples:

- Success: `ok=1 loaded=transport`
- Failure: `error=unknown_command`

