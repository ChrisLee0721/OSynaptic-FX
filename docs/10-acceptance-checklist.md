# 10 Acceptance Checklist

## A. Build & Test

- [ ] `build.ps1 -Compiler auto` 通过。
- [ ] `test.ps1 -Compiler auto` 通过。
- [ ] `test.ps1 -Matrix` 通过。
- [ ] `osfx-c99/build/quality_gate_report.md` 存在且三编译器为 `PASS`。

## B. Plugin Scope

- [ ] `plugin-list` 仅包含 `transport` / `test_plugin` / `port_forwarder`。
- [ ] 未纳入 `web/sql/dependency_manager/env_guard`。
- [ ] `port_forwarder` 规则添加、转发、统计、持久化可用。

## C. CLI

- [ ] `plugin-load` 可加载三类 scoped 插件。
- [ ] `plugin-cmd` 可成功路由到目标插件。
- [ ] `transport-status` / `test-plugin` / `port-forwarder` 子命令可用。

## D. Docs & Release

- [ ] `docs/` 文档库完整可读。
- [ ] `docs/08-release-notes.md` 已更新版本与日期。
- [ ] `docs/09-mirror-coverage-report.md` 与当前策略一致。
- [ ] `docs/12-config-quick-reference.md` 与当前脚本参数一致。
- [ ] `docs/13-performance-summary.md` 已同步最新基准结果。
- [ ] `docs/14-troubleshooting.md` 包含当前主要故障路径。
- [ ] `docs/15-release-playbook.md` 可直接执行发布流程。
- [ ] `docs/16-examples-cookbook.md` 场景示例可复制并通过抽检。
- [ ] `docs/17-glue-step-by-step.md` 可用于独立完成 glue 集成。
- [ ] 发布包包含 README、质量报告、Release Notes、镜像覆盖报告。

## E. 验收命令示例

```powershell
# 1) 基础构建与测试
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\build.ps1 -Compiler auto
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\test.ps1 -Compiler auto

# 2) 编译器矩阵
powershell -ExecutionPolicy Bypass -File .\osfx-c99\scripts\test.ps1 -Matrix

# 3) 检查质量报告
Get-Content .\osfx-c99\build\quality_gate_report.md

# 4) 抽检 CLI
.\osfx-c99\build\osfx_cli_cl.exe plugin-list
.\osfx-c99\build\osfx_cli_cl.exe transport-status
.\osfx-c99\build\osfx_cli_cl.exe test-plugin run component
```

## F. 验收结果判定

- 质量报告中 `clang/gcc/cl` 必须全部 `PASS`。
- CLI 抽检命令返回不应出现 `error=`。
- 若任一项失败，先修复后重新执行 A~E 全流程。

