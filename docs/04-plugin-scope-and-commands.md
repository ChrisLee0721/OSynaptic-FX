# 04 Plugin Scope And Commands

## 范围策略

### 纳入插件

- `transport`（lite）
- `test_plugin`（lite）
- `port_forwarder`（full）

### 排除插件

- `web`
- `sql`
- `dependency_manager`
- `env_guard`
- 其他非目标服务插件

## 命令矩阵

| Command | Description |
|---|---|
| `plugin-list` | 列出当前注册插件 |
| `plugin-load <name> [config]` | 加载插件 |
| `plugin-cmd <name> <cmd> [args...]` | 对指定插件下发命令 |
| `transport-status` | 快速查看 transport 状态 |
| `test-plugin ...` | 执行 test_plugin 命令 |
| `port-forwarder ...` | 执行 port_forwarder 命令 |

## 插件级命令

### transport

- `status`
- `dispatch <proto|auto> <hex_payload>`

### test_plugin

- `status`
- `run [suite]`

### port_forwarder

- `status`
- `stats`
- `list`
- `add-rule <name> <from_proto> <from_port|*> <to_proto> <to_port>`
- `remove-rule <name>`
- `enable-rule <name> <0|1>`
- `forward <from_proto> <from_port> <hex_payload>`
- `save [path]`
- `load [path]`

## 插件应用映射（具体用途）

| Plugin | Typical Use | Example |
|---|---|---|
| `transport` | 设备侧发包与链路 fallback | `plugin-cmd transport dispatch auto A1B2C3` |
| `test_plugin` | 现场健康检查、OTA 后快速验证 | `plugin-cmd test_plugin run component` |
| `port_forwarder` | 网关协议/端口转发桥接 | `port-forwarder add-rule r1 udp 8080 tcp 9000` |

说明：

- `transport` 更偏“发包执行面”。
- `test_plugin` 更偏“运行时自检面”。
- `port_forwarder` 更偏“网关转发策略面”。

## 快速示例

```powershell
# 1) 查看当前纳入插件
.\osfx-c99\build\osfx_cli_cl.exe plugin-list

# 2) 加载 scoped 插件
.\osfx-c99\build\osfx_cli_cl.exe plugin-load transport
.\osfx-c99\build\osfx_cli_cl.exe plugin-load test_plugin
.\osfx-c99\build\osfx_cli_cl.exe plugin-load port_forwarder

# 3) 通过 plugin-cmd 访问 transport
.\osfx-c99\build\osfx_cli_cl.exe plugin-cmd transport status
.\osfx-c99\build\osfx_cli_cl.exe plugin-cmd transport dispatch auto A1B2

# 4) 通过 plugin-cmd 访问 test_plugin
.\osfx-c99\build\osfx_cli_cl.exe plugin-cmd test_plugin run component

# 5) 通过 plugin-cmd 访问 port_forwarder
.\osfx-c99\build\osfx_cli_cl.exe plugin-cmd port_forwarder add-rule r1 udp 8080 tcp 9000
.\osfx-c99\build\osfx_cli_cl.exe plugin-cmd port_forwarder forward udp 8080 A1B2C3
```

## 预期输出示例

- `plugin-list` 输出包含 `transport,test_plugin,port_forwarder`。
- 加载成功通常返回 `ok=1 loaded=<name>`。
- 命令失败通常返回 `error=...`（如参数不足或命令不存在）。
- 若调用排除插件（如 `web`），应返回 `error=load_failed name=web` 或等价失败输出。

