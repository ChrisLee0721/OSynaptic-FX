# 18 Standardized Units Table

This document defines the mapping rules between user input units and standardized output units (pre-transmission calibration).

## 1. Scope and Source

- Standardization function: `osfx_standardize_value(...)`
- Data source: `OpenSynaptic/libraries` mirror (`osfx_library_catalog`)
- This table is a **subset of common units**, not a full dictionary.

## 2. User Input Fields (Unit-related)

In transmission interface `osfx_glue_encode_sensor_auto(...)`:

- User provides: `input_unit`
- System outputs: `out_unit` (standard unit)

> `cmd` is not a user-defined field (auto path is automatically determined by `fusion_state`).

## 3. Common Unit Mapping Table (Example)

| User Input Unit | Meaning | Canonical Output Unit | Conversion Rule |
|---|---|---|---|
| `kPa` | kilopascal | `Pa` | `value * 1000` |
| `Pa` | pascal | `Pa` | identity |
| `psi` | pound-force/inch^2 | `Pa` | library conversion |
| `mm[Hg]` | millimeter mercury | `Pa` | library conversion |
| `cel` | degree Celsius | `K` | `value + 273.15` |
| `degF` | degree Fahrenheit | `K` | `(value - 32) * 5/9 + 273.15` |
| `%` | percent | `%` | identity |

Notes:

- `library conversion` means the coefficient/offset comes from library data, not recommended to hardcode in business side.

## 4. Prefix Rules

If unit supports prefix and library marks it as prefixable, prefix will be automatically parsed:

- Example: `k` + base unit (e.g., `kPa`)
- Whether prefix is actually supported depends on unit metadata `can_take_prefix`

## 5. Input Constraints and Recommendations

- `input_unit` must be recognizable string.
- Recommended for business side to consistently use standard abbreviations (e.g., `kPa`, `cel`, `%`).
- If unit is unrecognized, current implementation will pass through unit as-is without conversion; production scenarios recommend adding whitelist validation at upper layer.

## 6. Pre-Transmission Check (Unit Dimension)

1. Check if `input_unit` is in allowed set (recommend whitelist).
2. Call `osfx_standardize_value(...)` to verify convertibility.
3. Compare output `out_unit` to meet business expectations (e.g., pressure unified to `Pa`).
4. Verify standardized value is within business legal range.

## 7. Reference Code Snippet

```c
double out_value = 0.0;
char out_unit[16];
if (!osfx_standardize_value(input_value, input_unit, &out_value, out_unit, sizeof(out_unit))) {
    // reject invalid input
}
```

## 8. Related Documentation

- Transmission field specification: `docs/17-glue-step-by-step.md`
- Examples cookbook: `docs/16-examples-cookbook.md`
- Architecture description: `docs/02-architecture.md`

