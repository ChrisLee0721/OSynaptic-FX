# 18 Standardized Units Table

本文档定义用户输入单位与标准化输出单位的映射规则（发送前口径）。

## 1. 范围与来源

- 标准化函数：`osfx_standardize_value(...)`
- 数据来源：`OpenSynaptic/libraries` 镜像（`osfx_library_catalog`）
- 本表为**常用单位子集**，不是全量词典。

## 2. 用户输入字段（单位相关）

发送接口 `osfx_glue_encode_sensor_auto(...)` 中：

- 用户提供：`input_unit`
- 系统输出：`out_unit`（标准单位）

> `cmd` 不是用户定义字段（auto 路径由 `fusion_state` 自动决定）。

## 3. 常用单位映射表（示例）

| User Input Unit | Meaning | Canonical Output Unit | Conversion Rule |
|---|---|---|---|
| `kPa` | kilopascal | `Pa` | `value * 1000` |
| `Pa` | pascal | `Pa` | identity |
| `psi` | pound-force/inch^2 | `Pa` | library conversion |
| `mm[Hg]` | millimeter mercury | `Pa` | library conversion |
| `cel` | degree Celsius | `K` | `value + 273.15` |
| `degF` | degree Fahrenheit | `K` | `(value - 32) * 5/9 + 273.15` |
| `%` | percent | `%` | identity |

说明：

- `library conversion` 表示系数/偏移来自库数据，不建议在业务侧硬编码。

## 4. 前缀规则

若单位支持前缀且库中标记可前缀化，则会自动解析前缀：

- 示例：`k` + base unit（如 `kPa`）
- 实际是否可前缀取决于单位元数据 `can_take_prefix`

## 5. 输入约束与建议

- `input_unit` 必须是可识别字符串。
- 建议业务侧统一使用标准缩写（如 `kPa`, `cel`, `%`）。
- 若单位未识别，当前实现会原样透传单位并不做转换；生产场景建议在上层加白名单校验。

## 6. 发送前检查（单位维度）

1. 检查 `input_unit` 是否在允许集（建议白名单）。
2. 调 `osfx_standardize_value(...)` 验证可转换性。
3. 比对输出 `out_unit` 是否符合业务预期（如压力统一到 `Pa`）。
4. 校验标准化后数值是否在业务合法区间。

## 7. 参考代码片段

```c
double out_value = 0.0;
char out_unit[16];
if (!osfx_standardize_value(input_value, input_unit, &out_value, out_unit, sizeof(out_unit))) {
    // reject invalid input
}
```

## 8. 联动文档

- 发送字段规范：`docs/17-glue-step-by-step.md`
- 示例手册：`docs/16-examples-cookbook.md`
- 架构说明：`docs/02-architecture.md`

