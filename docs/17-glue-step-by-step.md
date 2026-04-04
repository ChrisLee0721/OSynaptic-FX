# 17 Glue Step By Step

本文档给出 `osfx_glue` 的逐步代码说明，目标是让你把协议能力快速串成可运行链路。

## A. OpenSynaptic 标准数据格式定义（Normative）

**本节内容已迁移至独立规范文档，以保持主文档的聚焦。**

### 规范入口

详细的数据格式定义、字段约束、构帧规则、用户输入契约等内容，请参阅：

📄 **[docs/18-data-format-specification.md](18-data-format-specification.md)**

该文档包含以下核心内容：

| 主题 | 小节 | 说明 |
|------|------|------|
| **数据帧格式** | A.1 | 数据帧线格式与字段定义 |
| **命令字节表** | A.2 | 所有数据和控制命令的编码值 |
| **负载规范** | A.3-A.4 | 单传感器和控制帧结构 |
| **Secure 语义** | A.5 | 安全通道处理规则 |
| **一致性要求** | A.6-A.11 | 发送/接收端实现约束与错误处理 |
| **用户输入契约** | A.12 | 不同路径的输入字段定义（推荐、高级、标准兼容） |

### 本文档中的规范引用

在阅读本文档（17 Glue 集成流程）时，若遇到需要验证数据格式合法性的场景，可直接查阅 `18-data-format-specification.md` 中的对应小节。如：

- 编码传感器数据 → 见 A.12（用户输入契约）
- 验证控制帧结构 → 见 A.4（控制帧最小结构）
- 理解 Secure 数据处理 → 见 A.5（Secure 语义）

### 向下兼容说明

若你的代码仍引用旧版本中的 A 段内容，建议更新引用为：
```
docs/18-data-format-specification.md#{section}
```

例如：`18-data-format-specification.md#A.1` 对应原 `17-glue-step-by-step.md#A.1`

### A.1 [已迁移] 数据帧（DATA_*）线格式

详见 [18-data-format-specification.md#A.1](18-data-format-specification.md#A.1)

## 0. 目标

- 统一初始化：`fusion/security/id/runtime/plugin`
- 统一入口：编码、解码、握手分发、插件命令
- 适用于嵌入式主循环（轮询/事件驱动都可）

## 1. 头文件与上下文

```c
#include "osfx_core.h"

osfx_glue_ctx glue;
osfx_glue_config cfg;
```

说明：

- `osfx_glue_ctx` 是运行时总上下文。
- `osfx_glue_config` 是初始化配置（本地 AID、ID 池范围、租期、安全过期、回调）。

## 2. 配置默认值并按设备覆盖

```c
osfx_glue_default_config(&cfg);

cfg.local_aid = 500U;
cfg.id_start = 200U;
cfg.id_end = 1000U;
cfg.id_default_lease_seconds = 3600U;
cfg.secure_expire_seconds = 86400U;
```

建议：

- 设备侧固定 `local_aid` 时可直接写死。
- 作为服务端分配器时，重点设置 `id_start/id_end`。

## 3. 初始化胶水层

```c
int rc = osfx_glue_init(&glue, &cfg);
if (rc != OSFX_GLUE_OK) {
    return 1;
}
```

初始化内部完成：

- `osfx_fusion_state_init`
- `osfx_secure_store_init`
- `osfx_id_allocator_init`
- `osfx_protocol_matrix_init + register_defaults`
- `osfx_platform_runtime_init`
- 自动加载 `transport/test_plugin/port_forwarder`

## 4. 处理客户端 ID 申请（服务端路径）

```c
uint8_t id_req[3] = { OSFX_CMD_ID_REQUEST, 0x12, 0x34 };
osfx_hs_result hs;

rc = osfx_glue_process_packet(&glue, id_req, sizeof(id_req), 1710000000ULL, &hs);
if (rc == OSFX_GLUE_OK && hs.has_response) {
    // hs.response / hs.response_len -> 回发给客户端
}
```

预期：

- 成功分配：`hs.response[0] == OSFX_CMD_ID_ASSIGN`
- 分配失败：返回 NACK（常见 `ID_POOL_EXHAUSTED`）

## 5. 编码上行传感器（设备路径）

```c
uint8_t packet[256];
int packet_len = 0;
uint8_t out_cmd = 0;

rc = osfx_glue_encode_sensor_auto(
    &glue,
    1U,
    1710001000ULL,
    "TEMP",
    23.5,
    "cel",
    packet,
    sizeof(packet),
    &packet_len,
    &out_cmd
);
```

说明：

- `out_cmd` 会在 FULL/DIFF/HEART 之间自动切换。
- `packet` 可交给协议矩阵/网络层发送。

## 6. 解码下行数据包（接收路径）

```c
char sensor_id[32];
char unit[16];
double value = 0.0;
osfx_packet_meta meta;

rc = osfx_glue_decode_sensor_auto(
    &glue,
    packet,
    (size_t)packet_len,
    sensor_id,
    sizeof(sensor_id),
    &value,
    unit,
    sizeof(unit),
    &meta
);
```

输出：

- `sensor_id` / `value` / `unit`
- `meta.cmd/meta.source_aid/meta.timestamp_raw`

## 7. 插件命令统一调用

```c
char out[256];

osfx_glue_plugin_cmd(&glue, "transport", "status", "", out, sizeof(out));
osfx_glue_plugin_cmd(&glue, "test_plugin", "run", "component", out, sizeof(out));
osfx_glue_plugin_cmd(&glue, "port_forwarder", "list", "", out, sizeof(out));
```

常见用途：

- 运行时状态探活
- 端口转发规则运维
- 轻量自检触发

## 7.1 `transport` 插件具体应用（设备数据发出）

应用目标：设备把采样包按当前可用链路发出（auto 选择）。

步骤：

```c
char out[256];

// Step T1: 查看 transport 是否已初始化
osfx_glue_plugin_cmd(&glue, "transport", "status", "", out, sizeof(out));

// Step T2: 直接发送十六进制负载（协议 auto 选择）
osfx_glue_plugin_cmd(&glue, "transport", "dispatch", "auto A1B2C3", out, sizeof(out));
// 预期 out 包含: ok=1 used=<proto> bytes=3
```

适用场景：

- 设备开机自检后发首包。
- 主链路不可用时让 runtime 自动择优 fallback。

## 7.2 `test_plugin` 具体应用（现场健康检查）

应用目标：在设备现场快速做轻量回归，确认核心链路仍可工作。

步骤：

```c
char out[256];

// Step P1: 触发 component 自检
osfx_glue_plugin_cmd(&glue, "test_plugin", "run", "component", out, sizeof(out));
// 预期 out 包含: ok=1 suite=component

// Step P2: 查询累计执行状态
osfx_glue_plugin_cmd(&glue, "test_plugin", "status", "", out, sizeof(out));
// 预期 out 包含: initialized=1 total_runs/pass_runs
```

适用场景：

- OTA 后快速验收。
- 客户现场问题复现前先排除基础链路故障。

## 7.3 `port_forwarder` 具体应用（网关转发）

应用目标：网关把来源协议/端口的流量转发到目标协议/端口。

步骤：

```c
char out[256];

// Step F1: 添加规则 udp:8080 -> tcp:9000
osfx_glue_plugin_cmd(&glue, "port_forwarder", "add-rule", "r1 udp 8080 tcp 9000", out, sizeof(out));

// Step F2: 查看规则确认生效
osfx_glue_plugin_cmd(&glue, "port_forwarder", "list", "", out, sizeof(out));

// Step F3: 投递一帧并检查命中
osfx_glue_plugin_cmd(&glue, "port_forwarder", "forward", "udp 8080 A1B2", out, sizeof(out));
// 预期 out 包含: ok=1 route=tcp:9000 bytes=2

// Step F4: 查看统计
osfx_glue_plugin_cmd(&glue, "port_forwarder", "stats", "", out, sizeof(out));
```

适用场景：

- 现场异构设备协议桥接。
- 本地转发策略灰度验证（按规则开关启停）。

## 8. 主循环建议（伪代码）

```c
for (;;) {
    // A) 收包
    // recv(packet)

    // B) 交给 glue 做握手/控制面处理
    // if ctrl/data then osfx_glue_process_packet(...)
    // if hs.has_response -> send response

    // C) 周期采样并上报
    // osfx_glue_encode_sensor_auto(...)
    // send encoded packet

    // D) 可选：周期插件状态检查
    // osfx_glue_plugin_cmd(...)
}
```

## 9. 错误码速查

- `OSFX_GLUE_OK`：成功
- `OSFX_GLUE_ERR_ARG`：参数错误
- `OSFX_GLUE_ERR_RUNTIME`：运行时失败（分发/插件/初始化路径）
- `OSFX_GLUE_ERR_CODEC`：编码解码失败

## 10. 与文档联动

- 架构：`docs/02-architecture.md`
- 插件命令：`docs/04-plugin-scope-and-commands.md`
- 详细示例：`docs/16-examples-cookbook.md`
- 输入规范：`docs/19-input-specification.md`
- 发布流程：`docs/15-release-playbook.md`

## 11. 外部官方嵌入式库接入示例（MQTT / Zigbee / Matter / UDP）

本节说明：当你使用第三方官方嵌入式网络库时，如何把 `osfx_glue` 作为协议核心接入。

统一模式：

1. 入站（RX）：网络库收到 payload -> 调 `osfx_glue_process_packet(...)` 或 `osfx_glue_decode_sensor_auto(...)`。
2. 出站（TX）：先调 `osfx_glue_encode_sensor_auto(...)` 产出 packet -> 再交给网络库发送。
3. 控制面：若 `hs.has_response`，按原链路回发 response。

### 11.1 MQTT 官方库接入（示例）

适配定位：`MQTT` 非内置 driver，走外部库适配层。

```c
// MQTT RX callback
void on_mqtt_message(const uint8_t* payload, size_t len) {
    osfx_hs_result hs;
    int rc = osfx_glue_process_packet(&glue, payload, len, now_unix_ts(), &hs);
    if (rc == OSFX_GLUE_OK && hs.has_response) {
        // 通过 MQTT 回发到控制 topic
        mqtt_publish("osfx/ctrl/resp", hs.response, hs.response_len);
    }
}

// MQTT TX path
void publish_sensor_temp(double v) {
    uint8_t packet[256];
    int packet_len = 0;
    uint8_t out_cmd = 0;
    if (osfx_glue_encode_sensor_auto(&glue, 1U, now_unix_ts(), "TEMP", v, "cel",
                                     packet, sizeof(packet), &packet_len, &out_cmd) == OSFX_GLUE_OK) {
        mqtt_publish("osfx/data/up", packet, (size_t)packet_len);
    }
}
```

注意：

- MQTT topic 规划建议分离 `data` 与 `ctrl`。
- QoS 建议按业务时效与可靠性权衡。

### 11.2 Zigbee 官方库接入（示例）

适配定位：`Zigbee` 非内置 driver，走 cluster/endpoint payload 适配。

```c
// Zigbee RX indication
void on_zigbee_cluster_rx(const uint8_t* zb_payload, size_t zb_len) {
    osfx_hs_result hs;
    int rc = osfx_glue_process_packet(&glue, zb_payload, zb_len, now_unix_ts(), &hs);
    if (rc == OSFX_GLUE_OK && hs.has_response) {
        zigbee_send_cluster(hs.response, hs.response_len);  // 回发控制响应
    }
}

// Zigbee TX path
void zigbee_send_osfx_packet(void) {
    uint8_t packet[256];
    int packet_len = 0;
    uint8_t out_cmd = 0;
    if (osfx_glue_encode_sensor_auto(&glue, 1U, now_unix_ts(), "HUM", 55.0, "%",
                                     packet, sizeof(packet), &packet_len, &out_cmd) == OSFX_GLUE_OK) {
        zigbee_send_cluster(packet, (size_t)packet_len);
    }
}
```

注意：

- 需要确认 Zigbee 负载最大长度与分片策略。
- 若链路 MTU 较小，建议上层做分片重组。

### 11.3 Matter 官方库接入（示例）

适配定位：`Matter` 非内置 driver，走 endpoint/attribute 或 command payload 适配。

```c
// Matter command callback
void on_matter_command(const uint8_t* in_buf, size_t in_len) {
    osfx_hs_result hs;
    int rc = osfx_glue_process_packet(&glue, in_buf, in_len, now_unix_ts(), &hs);
    if (rc == OSFX_GLUE_OK && hs.has_response) {
        matter_send_command_resp(hs.response, hs.response_len);
    }
}

// Matter periodic report
void matter_report_temp(void) {
    uint8_t packet[256];
    int packet_len = 0;
    uint8_t out_cmd = 0;
    if (osfx_glue_encode_sensor_auto(&glue, 1U, now_unix_ts(), "TEMP", 24.2, "cel",
                                     packet, sizeof(packet), &packet_len, &out_cmd) == OSFX_GLUE_OK) {
        matter_send_attribute_blob(packet, (size_t)packet_len);
    }
}
```

注意：

- 建议把 OSynaptic packet 当作二进制 attribute/command payload。
- 注意 Matter 线程模型，避免在回调里做重负载操作。

### 11.4 UDP 官方库接入（示例）

适配定位：`UDP` 在当前项目中已有 protocol matrix 内置路径，也可直接用 socket 层接。

```c
// UDP RX loop
void on_udp_rx(const uint8_t* datagram, size_t n) {
    osfx_hs_result hs;
    int rc = osfx_glue_process_packet(&glue, datagram, n, now_unix_ts(), &hs);
    if (rc == OSFX_GLUE_OK && hs.has_response) {
        udp_sendto(peer_addr, hs.response, hs.response_len);
    }
}

// UDP TX
void udp_send_sensor(void) {
    uint8_t packet[256];
    int packet_len = 0;
    uint8_t out_cmd = 0;
    if (osfx_glue_encode_sensor_auto(&glue, 1U, now_unix_ts(), "PRESS", 101.3, "kPa",
                                     packet, sizeof(packet), &packet_len, &out_cmd) == OSFX_GLUE_OK) {
        udp_sendto(peer_addr, packet, (size_t)packet_len);
    }
}
```

注意：

- UDP 无连接且可能丢包，建议结合应用层重试/去重策略。
- 安全场景建议配合 `secure_session` 与时间戳防护。

