# 02 Architecture

## 分层结构

- `core facade`：`src/osfx_core_facade.c`，聚合标准化、编码、打包、解包。
- `protocol core`：`solidity`、`fusion_packet`、`fusion_state`、`template_grammar`。
- `security`：`secure_session`、`payload_crypto`、`handshake_dispatch`。
- `runtime`：`transporter_runtime`、`protocol_matrix`、`service_runtime`。
- `platform scoped plugins`：`plugin_transport`、`plugin_test`、`plugin_port_forwarder`。
- `platform adapter`：`platform_runtime` + `cli_lite` + `tools/osfx_cli_main.c`。
- `glue adapter`：`osfx_glue`，把编码、握手、ID、插件命令串成统一 API。

## 关键数据流

1. 传感器输入 -> 标准化 -> Base62 -> packet encode。
2. `fusion_state` 决定 FULL/DIFF/HEART 策略。
3. 可选安全会话加密后进入 protocol matrix。
4. `transport` 插件可按命令触发 auto/proto dispatch。
5. `port_forwarder` 插件可按规则从来源协议/端口转发到目标协议/端口。

## 详细控制流（服务端视角）

1. 收到控制包 `CMD=ID_REQUEST`（至少 3 字节，含 seq）。
2. `osfx_hs_classify_dispatch(...)` 从 dispatch ctx 读取 `id_allocator`。
3. 调用 `osfx_id_allocate(...)` 尝试分配 AID。
4. 成功：构建 `ID_ASSIGN` 响应；失败：构建 `NACK(ID_POOL_EXHAUSTED)`。
5. 调用方发送 `out_result.response` 回客户端。

## 详细数据流（设备侧视角）

1. `osfx_glue_encode_sensor_auto(...)` 读取本地 `local_aid` 与 `tx_state`。
2. 标准化 + Base62 + packet encode（FULL/DIFF/HEART 自动选择）。
3. 通过 protocol matrix 或上层发包器外送。
4. 回包/下行包进入 `osfx_glue_process_packet(...)`。
5. 若为数据包，执行时间戳防回放/乱序；若为控制包，进入 handshake 分发。

## 插件运行时关系

- `osfx_platform_runtime_init(...)` 负责注册 3 个 scoped 插件到 `service_runtime`。
- `plugin-list/load/cmd` 统一由 `osfx_cli_lite_run(...)` 路由。
- 插件状态通过 `service_runtime` 计数器和返回字符串暴露。

## 设计原则

- 固定范围：先保证协议核心与交付可控，不引入非目标服务。
- API 稳定：对外保持 `osfx_core.h` 统一入口。
- 可验证：每次改动必须通过 native/integration/CLI smoke。
- 编排解耦：通过 `osfx_glue` 统一调用路径，减少上层重复拼装逻辑。

