# 19 Input Specification

本文档定义 `osfx-c99` 在不同调用层级下的输入规范，重点回答：发送规则由谁提供、必须包含哪些字段。

## 1. 术语与分层

- **业务输入层**：调用 `osfx_glue_encode_sensor_auto(...)` 的输入。
- **线协议输入层**：通过网络接收后传给 `osfx_glue_process_packet(...)` 的二进制帧。
- **低层构帧层**：直接调用 `osfx_packet_encode_full/ex(...)` 的输入（高级用法）。

## 2. 业务输入层（OpenSynaptic 标准口径）

标准发送 API 口径（参考 `SEND_API_INDEX.md` / `SEND_API_REFERENCE.md`）：

- `transmit(sensors=[ [sensor_id, status, value, unit], ... ])`

在 `osfx-c99` 侧，等价到 `osfx_glue_encode_sensor_auto(...)`（单传感器）或多传感器封装。

### 2.1 必填字段（用户/业务侧提供）

| Field | Type | Required | Example | Notes |
|---|---|---|---|---|
| `tid` | `uint8_t` | Yes | `1` | 事务/通道标识 |
| `timestamp_raw` | `uint64_t` | Yes | `1710001000` | 建议单调递增 |
| `sensor_id` | `string` | Yes | `TEMP` | 不建议包含 `|` |
| `input_value` | `double` | Yes | `23.5` | 传感器原始值 |
| `input_unit` | `string` | Yes | `cel` / `kPa` | 见 `docs/18-standardized-units.md` |

### 2.2 系统自动生成字段（用户不应提供）

| Field | Source |
|---|---|
| `cmd` | 由 `fusion_state` 自动选择 FULL/DIFF/HEART |
| `route_count` | 固定 `1` |
| `source_aid` | `osfx_glue_ctx.local_aid` |
| `body` | 由标准化+Base62 编码生成 |
| `crc8` / `crc16` | 内部自动计算 |

### 2.3 发送规则

1. 先做单位标准化。
2. 再编码 `sensor_id|unit|b62_value`。
3. 再由状态机决定 `cmd`。
4. 最后组帧并计算 CRC。

## 3. 线协议输入层（接收后分发）

接口：`osfx_glue_process_packet(...)`

调用方必须提供完整二进制帧（已经是 OpenSynaptic 线格式）：

| Field | Required | Notes |
|---|---|---|
| `cmd` | Yes | 第 0 字节 |
| `route_count` | Yes | 第 1 字节，当前应为 `1` |
| `source_aid` | Yes | u32, big-endian |
| `tid` | Yes | u8 |
| `timestamp_raw` | Yes | u48, big-endian |
| `body` | Yes | 业务负载 |
| `crc8` | Yes | body CRC |
| `crc16` | Yes | 帧 CRC |

若缺字段/CRC 错误/时序异常，分发层必须拒绝并给出错误或 NACK。

## 4. 控制帧输入规范（常用）

### 4.1 `ID_REQUEST`

- 最小结构：`[cmd:1][seq:2]`
- 必填：`cmd=1`, `seq`
- 可选业务扩展：当前 C99 核心不要求附加 JSON 元数据

### 4.2 `TIME_REQUEST`

- 最小结构：`[cmd:1][seq:2]`

### 4.3 `SECURE_DICT_READY`

- 结构：`[cmd:1][aid:4][timestamp_raw:8]`

### 4.4 `SECURE_CHANNEL_ACK`

- 结构：`[cmd:1][aid:4]`

## 5. 低层构帧层（高级）

接口：`osfx_packet_encode_full(...)` / `osfx_packet_encode_ex(...)`

该模式下调用方需自行提供 `cmd/source_aid/tid/timestamp/body`，适用于：

- 网关协议桥接
- 测试工具
- 特殊控制面构帧

不建议普通业务代码直接使用此层。

## 6. 输入校验建议（发送前）

1. `sensor_id` 非空且长度在业务约束内。
2. `input_unit` 在白名单（见 `docs/18-standardized-units.md`）。
3. `timestamp_raw` 不回拨（至少单设备单调）。
4. 缓冲区容量足够（避免截断）。
5. 错误立即拒发，禁止带错发送。

## 7. 典型错误与处理

- `OSFX_GLUE_ERR_ARG`：字段缺失/非法。
- `OSFX_GLUE_ERR_CODEC`：标准化或编码失败。
- `OSFX_GLUE_ERR_RUNTIME`：运行时依赖不可用（会话/路由/插件）。

## 8. 参考文档

- 线格式规范：`docs/17-glue-step-by-step.md`
- 单位规范：`docs/18-standardized-units.md`
- 详细示例：`docs/16-examples-cookbook.md`

## 8.1 基础 Config 与编译期生效

`osfx-c99` 支持用基础配置文件驱动编译期开关：

- 配置源：`osfx-c99/Config.json`
- 生成头：`osfx-c99/include/osfx_build_config.h`
- 构建配置头：`include/osfx_build_config.h`（仓库内已提交）

当前已编译期生效的开关（示例）：

- `OSFX_CFG_PLUGIN_TRANSPORT`
- `OSFX_CFG_PLUGIN_TEST_PLUGIN`
- `OSFX_CFG_PLUGIN_PORT_FORWARDER`
- `OSFX_CFG_AUTOLOAD_TRANSPORT`
- `OSFX_CFG_AUTOLOAD_TEST_PLUGIN`
- `OSFX_CFG_AUTOLOAD_PORT_FORWARDER`

`payload_switches` 会生成对应宏（`OSFX_CFG_PAYLOAD_*`），并已接入多传感器模板编码路径（`osfx_core_encode_multi_sensor_packet_auto(...)`）：

- `DeviceId` / `DeviceStatus` / `Timestamp` / `SubTemplateId`
- `SensorId` / `SensorStatus` / `PhysicalAttribute` / `NormalizedValue`
- `GeohashId` / `SupplementaryMessage` / `ResourceUrl`

## 9. 完整示例：10 传感器发送（包含所有字段）

本示例给出“业务输入层 + 线协议层”的完整字段视图，用于联调对照。

### 9.1 业务输入层（用户提供）

调用：`osfx_glue_encode_multi_sensor_packet_auto(...)`（或上层等价封装）

> 下述为 **OpenSynaptic 标准发送口径**（4元组）。

```text
tid = 2
timestamp_raw = 1710001234
node_id = NODE_A
node_state = ONLINE

sensors[10] = {
  [TEMP1, OK, 23.5, cel],
  [TEMP2, OK, 24.1, cel],
  [PRESS1, OK, 101.3, kPa],
  [PRESS2, OK, 99.8, kPa],
  [HUM1, OK, 55.0, %],
  [HUM2, OK, 57.2, %],
  [FLOW1, OK, 12.7, L/min],
  [FLOW2, OK, 13.1, L/min],
  [VIB1, OK, 0.82, g],
  [VIB2, OK, 0.79, g]
}
```

对应的 **C 可编译写法**（标准字段）如下：

```c
osfx_core_sensor_input sensors[10] = {
    {"TEMP1", "OK", 23.5, "cel", "", "", ""},
    {"TEMP2", "OK", 24.1, "cel", "", "", ""},
    {"PRESS1", "OK", 101.3, "kPa", "", "", ""},
    {"PRESS2", "OK", 99.8, "kPa", "", "", ""},
    {"HUM1", "OK", 55.0, "%", "", "", ""},
    {"HUM2", "OK", 57.2, "%", "", "", ""},
    {"FLOW1", "OK", 12.7, "L/min", "", "", ""},
    {"FLOW2", "OK", 13.1, "L/min", "", "", ""},
    {"VIB1", "OK", 0.82, "g", "", "", ""},
    {"VIB2", "OK", 0.79, "g", "", "", ""}
};
```

扩展模式（非标准口径）可使用：

- `geohash_id`
- `supplementary_message`
- `resource_url`

但该模式要求两端解析器一致支持，不建议用于“严格 OpenSynaptic 兼容”链路。

字段对齐关系（扩展模式）：

- 伪语法里的 `id/state/value/unit/geohash/msg/url`
- 对应 C 字段 `sensor_id/sensor_state/value/unit/geohash_id/supplementary_message/resource_url`

### 9.2 系统自动生成/决定字段

- `cmd`：由 `fusion_state` 自动决定（首帧通常为 `DATA_FULL=63`）。
- `route_count`：固定 `1`。
- `source_aid`：来自 `osfx_glue_ctx.local_aid`（例如 `500`）。
- 标准化输出单位与 `b62_value`：由标准化+Base62 自动生成。
- `crc8`、`crc16`：自动计算并写入。

### 9.3 多传感器 body（模板语法）

严格兼容语法（推荐）：

```text
node_id.node_state.ts|sensor_id>sensor_state.sensor_unit:sensor_value|...
```

扩展语法（仅扩展模式）：

```text
node_id.node_state.ts|sensor_id>sensor_state.sensor_unit:sensor_value[#geohash][!msg][@url]|...
```

10 传感器示例（严格兼容，符号化展示）：

```text
NODE_A.ONLINE.1710001234|
TEMP1>OK.K:<b62_temp1>|TEMP2>OK.K:<b62_temp2>|
PRESS1>OK.Pa:<b62_press1>|PRESS2>OK.Pa:<b62_press2>|
HUM1>OK.%:<b62_hum1>|HUM2>OK.%:<b62_hum2>|
FLOW1>OK.<std_unit_flow>:<b62_flow1>|FLOW2>OK.<std_unit_flow>:<b62_flow2>|
VIB1>OK.<std_unit_vib>:<b62_vib1>|VIB2>OK.<std_unit_vib>:<b62_vib2>|
```

说明：

- 温度 `cel` 标准化后单位为 `K`。
- 压力 `kPa` 标准化后单位为 `Pa`。
- 其它单位按库规则标准化（见 `docs/18-standardized-units.md`）。
- 扩展模式下字段顺序固定为：`#geohash` -> `!supplementary_message` -> `@resource_url`。

### 9.4 线协议层完整字段（最终帧）

| Field | Value Example | Provided By |
|---|---|---|
| `cmd` | `63` (`DATA_FULL`) | system |
| `route_count` | `1` | system |
| `source_aid` | `500` | system (`glue.local_aid`) |
| `tid` | `2` | user |
| `timestamp_raw` | `1710001234` | user |
| `body` | 模板编码后的完整字符串 | mixed（user + system） |
| `crc8` | `<auto_crc8>` | system |
| `crc16` | `<auto_crc16>` | system |

> 注意：`<auto_crc8>` 和 `<auto_crc16>` 不由用户填写，必须由编码器根据真实 body 自动计算。

### 9.5 发送规则落地结论

- 用户负责“业务字段”（`tid/timestamp/node/sensors`）。
- 系统负责“协议字段”（`cmd/aid/crc/线格式拼装`）。
- 用户不应手工定义 `cmd`（auto 路径）。

### 9.6 实测样例（10 传感器，按当前 `Config.json` 编译）

以下样例来自当前构建实测输出（`cmd=63`，首帧 FULL）：

```text
cmd=63
packet_len=303
source_aid=500
tid=2
timestamp_raw=1710001234
body_len=287
body=NODE_A.ONLINE.1710001234|TEMP1>OK.NA:crIM!msg_temp1|TEMP2>OK.NA:cthy!msg_temp2|PRESS1>OK.NA:16yrzG!msg_press1|PRESS2>OK.NA:15xvoc!msg_press2|HUM1>OK.NA:2j4Y!msg_hum1|HUM2>OK.NA:2oNO!msg_hum2|FLOW1>OK.NA:x2o!msg_flow1|FLOW2>OK.NA:y4U!msg_flow2|VIB1>OK.NA:8!msg_vib1|VIB2>OK.NA:7!msg_vib2|
packet_hex=3F01000001F402000065EC8C524E4F44455F412E4F4E4C494E452E313731303030313233347C54454D50313E4F4B2E4E413A6372494D216D73675F74656D70317C54454D50323E4F4B2E4E413A63746879216D73675F74656D70327C5052455353313E4F4B2E4E413A313679727A47216D73675F7072657373317C5052455353323E4F4B2E4E413A313578766F63216D73675F7072657373327C48554D313E4F4B2E4E413A326A3459216D73675F68756D317C48554D323E4F4B2E4E413A326F4E4F216D73675F68756D327C464C4F57313E4F4B2E4E413A78326F216D73675F666C6F77317C464C4F57323E4F4B2E4E413A793455216D73675F666C6F77327C564942313E4F4B2E4E413A38216D73675F766962317C564942323E4F4B2E4E413A37216D73675F766962327CCA12E3
```

字段解读（对应当前 `payload_switches`）：

- `PhysicalAttribute=false` => `sensor_unit` 输出为 `NA`
- `SupplementaryMessage=true` => 每个传感器值后出现 `!msg_xxx`
- `GeohashId=false` => 不出现 `#geohash`
- `ResourceUrl=false` => 不出现 `@url`

若要在 body 中看到 geohash/url，请将对应开关设为 `true` 后重新编译。

