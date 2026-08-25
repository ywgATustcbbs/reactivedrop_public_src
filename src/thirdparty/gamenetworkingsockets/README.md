# Vendored GameNetworkingSockets build input

本目录只用于 `game/gns_wrapper/asrd_gns_wrapper.vcxproj`：它包含 x86
`x86-windows-static` 的 GNS、protobuf、Abseil 和 utf8 静态库及公共头文件。

该包由上游 GameNetworkingSockets `505c697d0abef5da2ff3be35aa4ea3687597c3e9`
通过 vcpkg 构建，配置为 `USE_CRYPTO=BCrypt`、`USE_CRYPTO25519=BCrypt`、
`Protobuf_USE_STATIC_LIBS=ON`、静态 CRT。游戏客户端和服务端不得直接包含这里的
GNS 头文件或链接这里的库；只有包装 DLL 工程可以使用它们。

`bin/` 和 `bin/debug/` 保持为空。包装 DLL 的运行时依赖只有 Windows 自带的
BCrypt、Winsock、WinMM、IP Helper 等系统组件。
