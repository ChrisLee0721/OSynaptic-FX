# 08 Release Notes

## Release

- Version: `v1.0.0-p5` (建议标签)
- Date: `2026-04-04`
- Package: `osfx-c99` static library + scoped platform plugins + CLI lite

## Highlights

- 完成 `P0~P4` 核心里程碑。
- 插件策略收敛为：`transport` lite、`test_plugin` lite、`port_forwarder` full。
- 新增独立 CLI 入口（`tools/osfx_cli_main.c`）与轻量命令路由。
- 质量闸门支持 `clang/gcc/cl` 编译器矩阵。

## Quality Gate

- 报告文件：`osfx-c99/build/quality_gate_report.md`
- 当前结果：三编译器 Native/Integration/CLI Smoke 全 `PASS`。

## Compatibility Notes

- 目标定位为协议核心与受控平台面，不覆盖 OpenSynaptic 全服务生态。
- 显式排除 `web/sql/dependency_manager/env_guard`。

## Known Non-Blocking Items

- 驱动行为语义仍可继续深度优化（非发布阻塞）。
- 部分平台级运营能力留待后续迭代。

