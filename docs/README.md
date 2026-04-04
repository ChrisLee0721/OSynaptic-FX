# osfx-c99 Documentation Library

本目录是 `osfx-c99` 的完整交付文档库，覆盖架构、API、插件范围、CLI、质量闸门、发布与验收。

## 阅读入口

- 快速导航：`docs/SUMMARY.md`
- 工程总览：`docs/01-overview.md`
- 架构说明：`docs/02-architecture.md`
- API 索引：`docs/03-api-index.md`
- 插件范围与命令：`docs/04-plugin-scope-and-commands.md`
- `port_forwarder` 全量说明：`docs/05-port-forwarder.md`
- CLI 轻量化使用：`docs/06-cli.md`
- 质量闸门与编译器矩阵：`docs/07-quality-gate-and-compiler-matrix.md`
- Release Notes：`docs/08-release-notes.md`
- 镜像覆盖报告：`docs/09-mirror-coverage-report.md`
- 发布验收清单：`docs/10-acceptance-checklist.md`
- 基准方法与口径：`docs/11-benchmark-method.md`
- 配置速查：`docs/12-config-quick-reference.md`
- 性能结果摘要：`docs/13-performance-summary.md`
- 故障排障：`docs/14-troubleshooting.md`
- 发布流程：`docs/15-release-playbook.md`
- 详细示例手册：`docs/16-examples-cookbook.md`
- Glue 逐步代码说明：`docs/17-glue-step-by-step.md`
- 标准化单位表：`docs/18-standardized-units.md`
- 输入规范（发送规则与字段）：`docs/19-input-specification.md`
- 版本变更日志：`docs/CHANGELOG.md`

## 当前文档基线

- 插件策略：仅纳入 `transport`（lite）、`test_plugin`（lite）、`port_forwarder`（full）。
- 显式排除：`web`、`sql`、`dependency_manager`、`env_guard` 及其他服务插件。
- 质量门：支持 `clang/gcc/cl` 矩阵验证，报告产物为 `osfx-c99/build/quality_gate_report.md`。

