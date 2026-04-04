# 05 Port Forwarder (Full)

## 能力边界

`port_forwarder` 在当前范围内是 full 实现，覆盖规则增删改查、命中统计、转发执行、规则持久化与加载。

## 规则模型

- Rule 字段：`name`, `from_proto`, `from_port|*`, `to_proto`, `to_port`, `enabled`, `hit_count`。
- 匹配逻辑：按注册顺序扫描，匹配到第一条即执行转发。
- 端口策略：`*` 表示来源端口通配。

## 运行统计

- 插件级：`total_forwarded`, `total_dropped`
- 规则级：`hit_count`

## 命令说明

- `status`：返回初始化状态与总转发/丢弃统计。
- `stats`：返回统计摘要。
- `list`：返回规则列表。
- `add-rule`：新增或覆盖同名规则。
- `remove-rule`：删除规则。
- `enable-rule`：启停规则。
- `forward`：输入来源协议+端口+hex 数据，执行匹配转发。
- `save/load`：规则持久化。

## 持久化格式

- 文件为逐行文本记录，字段逗号分隔。
- 推荐将路径固定在发布目录下并纳入备份策略。

## 典型流程

1. `plugin-load port_forwarder`
2. `port-forwarder add-rule r1 udp 8080 tcp 9000`
3. `port-forwarder forward udp 8080 A1B2C3`
4. `port-forwarder stats`
5. `port-forwarder save <path>`

## 端到端示例（可复制）

```powershell
# 初始化与加载
.\osfx-c99\build\osfx_cli_cl.exe plugin-load port_forwarder

# 新增规则: udp:8080 -> tcp:9000
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder add-rule r1 udp 8080 tcp 9000

# 查看规则
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder list

# 触发转发（十六进制负载）
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder forward udp 8080 A1B2C3D4

# 查看统计
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder stats
```

## 通配端口示例

```powershell
# 将任意来源端口的 udp 数据转发到 tcp:9100
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder add-rule any_udp udp * tcp 9100
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder forward udp 5001 AABB
```

## 持久化示例

```powershell
# 保存规则
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder save E:\OSynapptic-FX\osfx-c99\build\pf_rules.txt

# 重新加载规则
.\osfx-c99\build\osfx_cli_cl.exe port-forwarder load E:\OSynapptic-FX\osfx-c99\build\pf_rules.txt
```

## 常见失败与处理

- 现象：`error=usage add-rule ...`
- 原因：参数数量不足或顺序错误。
- 处理：按 `add-rule <name> <from_proto> <from_port|*> <to_proto> <to_port>` 重试。

- 现象：`error=no_match_or_emit_failed`
- 原因：无匹配规则，或 emit 回调发送失败。
- 处理：先 `list` 校验规则，再检查 `from_proto/from_port` 与入参是否一致。

