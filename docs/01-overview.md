# 01 Overview

## 项目定位

`osfx-c99` 是 OpenSynaptic 协议核心的 C99 嵌入式实现，目标是提供可静态链接、可移植、可验证的核心能力，不追求全平台服务生态 1:1 复制。

## 已实现范围

- 编解码与校验：Base62、CRC8、CRC16/CCITT。
- 数据包：FULL/DIFF/HEART 编码与最小元数据解码。
- 融合状态机：自动策略切换与回放路径。
- 安全面：会话状态、密钥派生、时间戳单调防护、持久化。
- 运行时：transporter runtime、protocol matrix、service runtime。
- 插件体系（本阶段）：`transport` lite、`test_plugin` lite、`port_forwarder` full。
- CLI：轻量命令路由与独立入口 `tools/osfx_cli_main.c`。

## 阶段状态

- `P0`：核心闭环完成。
- `P1`：库数据驱动镜像完成（Units/Prefixes/Symbols）。
- `P2`：插件/服务运行时镜像完成。
- `P3`：安全控制面边界处理完成。
- `P4`：ID 租约策略增强完成。
- 当前收口方向：发布文档、镜像覆盖与持续优化。

## 非目标（当前）

- 不纳入 `web/sql/dependency_manager/env_guard`。
- 不实现 OpenSynaptic 全量服务生态与复杂运维面。

