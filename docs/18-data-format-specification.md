# 18 OpenSynaptic 标准数据格式定义（Normative）

本文档给出 `osfx-c99` 当前实现使用的标准数据格式严格定义。除非另有说明，所有多字节整型均为 **大端序（big-endian）**。

## A.1 数据帧（DATA_*）线格式

适用命令：`DATA_FULL(63)`、`DATA_DIFF(170)`、`DATA_HEART(127)` 及其 secure 变体（`64/171/128`）。

| Offset | Length | Field | Definition |
|---:|---:|---|---|
| 0 | 1 | `cmd` | 命令字节 |
| 1 | 1 | `route_count` | 当前实现固定为 `1` |
| 2 | 4 | `source_aid` | 发送方 AID（u32） |
| 6 | 1 | `tid` | 事务/通道标识（u8） |
| 7 | 6 | `timestamp_raw` | 原始时间戳（u48） |
| 13 | N | `body` | 业务负载 |
| 13+N | 1 | `crc8` | 对 `body` 的 CRC8 |
| 14+N | 2 | `crc16` | 对前序所有字节（含 `crc8`）的 CRC16/CCITT |

接收侧必须先验证 `crc16`，失败应拒绝；若时间戳防护开启，还应执行 replay/out-of-order 校验。

## A.2 命令字节定义

| 类别 | Command | Value |
|---|---|---:|
| Data | `DATA_FULL` | 63 |
| Data | `DATA_FULL_SEC` | 64 |
| Data | `DATA_DIFF` | 170 |
| Data | `DATA_DIFF_SEC` | 171 |
| Data | `DATA_HEART` | 127 |
| Data | `DATA_HEART_SEC` | 128 |
| Control | `ID_REQUEST` | 1 |
| Control | `ID_ASSIGN` | 2 |
| Control | `HANDSHAKE_ACK` | 5 |
| Control | `HANDSHAKE_NACK` | 6 |
| Control | `PING` | 9 |
| Control | `PONG` | 10 |
| Control | `TIME_REQUEST` | 11 |
| Control | `TIME_RESPONSE` | 12 |
| Control | `SECURE_DICT_READY` | 13 |
| Control | `SECURE_CHANNEL_ACK` | 14 |

## A.3 单传感器业务负载规范

`osfx_core_encode_sensor_packet*` 生成的 body 规范：

```text
sensor_id|unit|b62_value
```

约束：

- `sensor_id`：传感器标识字符串。
- `unit`：标准化后的单位（如 `Pa`、`K`）。
- `b62_value`：`round(standardized_value * 10000)` 后的 `int64` Base62 编码结果。

## A.4 控制帧最小结构（常用）

- `ID_REQUEST`：`[cmd:1][seq:2]`（最小 3 字节）
- `ID_ASSIGN`：`[cmd:1][seq:2][assigned_id:4]`
- `HANDSHAKE_NACK`：`[cmd:1][seq:2][reason:utf8-bytes]`
- `TIME_REQUEST`：`[cmd:1][seq:2]`
- `TIME_RESPONSE`：`[cmd:1][seq:2][unix_ts:8]`

## A.5 Secure 语义

- secure data cmd 必须映射到对应 base data cmd 进行语义处理。
- 若会话未建立而收到 secure data，接收侧应拒绝（`NO_SESSION` 语义）。
- 时间戳检查返回：`ACCEPT / REPLAY / OUT_OF_ORDER`，后两者必须拒绝。

## A.6 实现一致性要求

- 发送端必须按本节顺序与编码规则构帧。
- 接收端必须按本节校验顺序验帧（长度/CRC/时序）。
- 任一字段不满足约束时，必须返回错误或 NACK，不应静默吞包。

## A.7 发送数据必填字段要求（Sender MUST）

发送侧在构造 `DATA_*` 帧时必须提供以下字段，缺一不可：

- `cmd`：必须是已定义数据命令之一（`63/170/127` 或 secure 变体）。
- `route_count`：当前实现必须写 `1`。
- `source_aid`：必须为有效设备 ID（u32）。
- `tid`：必须填写（u8）。
- `timestamp_raw`：必须填写（u48，建议单调递增）。
- `body`：必须存在，且按业务负载规范编码。
- `crc8`：必须基于 `body` 计算后写入。
- `crc16`：必须基于前序全部字节计算后写入。

## A.8 字段取值与边界

- `cmd`：仅允许命令表中定义值；未知值不得发送。
- `route_count`：固定 `1`。
- `source_aid`：`0..4294967295`（工程上建议避免 `0`）。
- `tid`：`0..255`。
- `timestamp_raw`：`0..2^48-1`（线格式）；建议采用秒级或毫秒级统一口径。
- `body_len`：必须满足实现缓冲区上限；超过上限必须拒发。

## A.9 构帧顺序（发送侧唯一顺序）

1. 先完成业务数据标准化与 body 编码。
2. 对 `body` 计算 `crc8`。
3. 按 `A.1` 定义写入头字段（`cmd/route_count/source_aid/tid/timestamp_raw`）。
4. 拼接 `body + crc8`。
5. 对当前整段（头 + body + crc8）计算 `crc16` 并追加。
6. 得到最终可发送二进制帧。

> 任何偏离该顺序的实现都可能导致接收端 CRC 校验失败。

## A.10 发送前校验清单（Pre-Send Checklist）

发送前至少执行以下检查：

- 命令合法性检查（`cmd` 是否在定义集合中）。
- 时间戳有效性检查（空值/回拨/异常跳变）。
- 缓冲区容量检查（避免截断写入）。
- `crc8/crc16` 自检（可选二次计算比对）。
- secure 场景会话检查（`dict_ready/key_set` 是否满足）。

任一检查失败必须中止发送并返回错误。

## A.11 错误处理规范（发送侧）

- 字段缺失/非法：返回参数错误（建议映射 `OSFX_GLUE_ERR_ARG`）。
- 编码失败：返回编解码错误（建议映射 `OSFX_GLUE_ERR_CODEC`）。
- 运行时依赖失败（会话/插件/路由不可用）：返回运行时错误（建议映射 `OSFX_GLUE_ERR_RUNTIME`）。
- 禁止"带错发送"：发生错误时必须拒发，不得发送半成品或无效帧。

## A.12 用户输入数据契约（User Input Contract）

本节定义"业务侧/用户侧需要提供什么"，避免把线格式字段误认为都由用户填写。

### A.12.1 `osfx_glue_encode_sensor_auto(...)` 路径（推荐）

用户必须提供：

- `tid`
- `timestamp_raw`
- `sensor_id`
- `input_value`
- `input_unit`

用户**不需要**提供（由系统自动生成或内部维护）：

- `cmd`（由 `fusion_state` 自动决定 FULL/DIFF/HEART）
- `route_count`（固定为 `1`）
- `source_aid`（来自 `glue.local_aid`）
- `crc8` / `crc16`（内部自动计算）

### A.12.2 低层 `osfx_packet_encode_full/ex` 路径（高级）

若你直接调用低层打包 API，则 `cmd/source_aid/tid/timestamp/body` 由调用方显式提供；
此模式仅建议给协议网关或测试工具使用，业务侧默认走 `osfx_glue_encode_sensor_auto(...)`。

### A.12.3 用户输入最小合法集（单传感器）

| Field | Required | Example | Notes |
|---|---|---|---|
| `sensor_id` | Yes | `TEMP` | 建议短标识，避免分隔符 `\|` |
| `input_value` | Yes | `23.5` | 有效浮点数 |
| `input_unit` | Yes | `cel` / `kPa` / `%` | 见 `docs/18-standardized-units.md` |
| `timestamp_raw` | Yes | `1710001000` | 建议单调递增 |
| `tid` | Yes | `1` | `0..255` |

#### A.12.4 用户输入最大合法集（业务侧）

> 最大合法集指"在当前实现与配置下，业务侧可完整提供且被编码链路消费的字段集合"。

单传感器路径（`osfx_glue_encode_sensor_auto(...)`）最大合法集：

| Field | Required | Example | Notes |
|---|---|---|---|
| `sensor_id` | Yes | `TEMP` | 传感器标识 |
| `input_value` | Yes | `23.5` | 原始测量值 |
| `input_unit` | Yes | `cel` / `kPa` | 标准化输入单位 |
| `timestamp_raw` | Yes | `1710001000` | 建议单调递增 |
| `tid` | Yes | `1` | 子模板/事务槽位 |

多传感器路径（`osfx_core_encode_multi_sensor_packet_auto(...)`）最大合法集：

| Field | Required | Example | Notes |
|---|---|---|---|
| `node_id` | Yes | `NODE_A` | 设备标识 |
| `node_state` | Yes | `ONLINE` | 设备状态 |
| `timestamp_raw` | Yes | `1710001234` | 时间戳 |
| `tid` | Yes | `2` | 子模板/事务槽位 |
| `sensor[i].sensor_id` | Yes | `TEMP1` | 每个传感器必填 |
| `sensor[i].sensor_state` | Yes | `OK` | 每个传感器必填 |
| `sensor[i].value` | Yes | `23.5` | 每个传感器必填 |
| `sensor[i].unit` | Yes | `cel` | 每个传感器必填 |
| `sensor[i].geohash_id` | Optional | `wx4g0ec1` | `GeohashId=true` 时编码进模板 |
| `sensor[i].supplementary_message` | Optional | `msg_temp1` | `SupplementaryMessage=true` 时编码进模板 |
| `sensor[i].resource_url` | Optional | `https://r/1` | `ResourceUrl=true` 时编码进模板 |

#### A.12.5 需要服务端与客户端同时开启的项（强一致）

以下开关若两端不一致，会导致字段语义不一致（甚至解码后业务含义错误），建议服务端与客户端同时开启或同时关闭：

- `DeviceId`（node/source identity 语义）
- `DeviceStatus`（node_state 语义）
- `Timestamp`（时间序语义）
- `SubTemplateId`（`tid` 语义）
- `SensorId`（传感器映射语义）
- `SensorStatus`（状态语义）
- `PhysicalAttribute`（单位字段语义）
- `NormalizedValue`（数值字段语义）

安全与控制面相关项也建议双端一致：

- secure 会话启用策略（`secure_session` 相关流程）
- 时间戳防回放/乱序策略
- ID 分配控制帧处理策略（`ID_REQUEST/ID_ASSIGN/NACK`）

#### A.12.6 扩展字段接线状态（当前）

以下字段已接入多传感器模板编码链路，并受 `payload_switches` 编译期开关控制：

- `GeohashId`
- `SupplementaryMessage`
- `ResourceUrl`

建议服务端与客户端保持一致开关，避免出现"发送侧有字段、接收侧按空字段处理"的语义偏差。

#### A.12.7 OpenSynaptic 严格兼容建议（重要）

若目标是与原版 OpenSynaptic 发送 API 严格兼容，建议使用标准 4 元组输入口径：

```text
[sensor_id, sensor_status, value, unit]
```

并在兼容链路中关闭扩展字段开关：

- `GeohashId = false`
- `SupplementaryMessage = false`
- `ResourceUrl = false`

这样可避免扩展模板字段导致对端解析器不兼容。

## 联动文档

- 整合说明：`docs/17-glue-step-by-step.md` 第 0-11 节
- 数据处理管道：`../DATA_FORMATS_SPEC.md`
- 标准化单位：`docs/19-input-specification.md` 或相应标准化单位文档
- 实现示例：`docs/16-examples-cookbook.md`

