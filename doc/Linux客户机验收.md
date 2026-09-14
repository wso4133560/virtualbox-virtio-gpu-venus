# Linux 客户机启动与 VirtIO-GPU 验收

此入口验证真实 Linux 启动、SSH、PCI/virtio 驱动绑定和 DRM 节点。Venus 模式还要求 Linux 接受 host-visible 共享内存窗口。它不把这些结果等同于客户机 Vulkan 应用执行成功。

## 固定环境与复现

- 客户机：Ubuntu 24.04.5，官方 20260911 amd64 cloud image；内核 `6.8.0-139-generic`。
- 镜像来源与 SHA256 固定在 `tools/linux-test-image.json`；原始镜像为 625256960 字节。
- 下载脚本需要 PowerShell 7，支持断点续传；VM 验证入口已在 Windows PowerShell 5.1 中执行。
- 镜像转换使用已有的 `qemu-img`，当前机器为 `C:\msys64\ucrt64\bin\qemu-img.exe`（10.2.2），其他位置通过 `-QemuImgPath` 指定。
- 复用已注册、已签名且 VBoxSup 可用的 VirtualBox 运行环境。脚本不安装服务、不切换系统启动安全设置。

在仓库根目录的 PowerShell 7 运行：

```powershell
$image = .\tools\get-linux-test-image.ps1
.\tools\test-linux-virtio-gpu.ps1 -ImagePath $image -GpuBackend software
.\tools\test-linux-virtio-gpu.ps1 -ImagePath $image -GpuBackend venus
```

每次运行创建独立名称的临时 VM、20GB 动态 VDI、NoCloud VISO 和 SSH 密钥。默认配置为单 vCPU、4GB 内存、BIOS，SSH 只绑定 `127.0.0.1:2222`；无密码登录入口。默认单核配置用于本机可重复的功能验证，不是性能配置。

运行结束会定向关闭并注销本次 VM，删除 VDI、种子介质和认证密钥。已校验的原始镜像保留在 `.build/images` 缓存；日志保留在 `.build/windows/virtio-linux-*`。可用 `-ReportDirectory` 指定一个尚不存在的报告目录，避免覆盖旧证据。

从运行时 ZIP 的 `tools` 目录执行时会自动选用包内 `bin`；也可通过 `-RuntimeDirectory` 指定构建目录。仍需本机提供 QEMU 镜像转换工具和 OpenSSH。

## 验收条件

`report.json` 的 `passed=true` 必须同时满足：

1. 镜像字节数和 SHA256 与固定记录一致。
2. VM 在等待期间保持 RUNNING；cloud-init 完成标记能通过 SSH 读取。
3. DRM 字符设备存在，且对应 PCI 设备下的 virtio 子设备绑定 `virtio_gpu`。PCI 层的驱动名本来就是 `virtio-pci`，不能误用它判断功能驱动缺失。
4. Venus 模式的来宾内核日志包含 `+host_visible`。
5. 本次 VM 清理没有错误。

`guest.log` 保存 Linux 版本、`lspci -nnk/-vv`、DRM 节点、virtio 驱动链接及内核日志；`serial.log` 保存从启动到关闭的串口输出；`VBox.log` 保存宿主设备和 VM 日志。JSON 记录 CPU 配置、启动就绪耗时、宿主 Vulkan 设备和运行文件哈希。`guestVulkanVerified` 当前始终为 false。

## 2026-09-14 协议修复证据

修复前，真实 Linux 报告 `shm cap with bad size ... size: 28`，并显示 `-host_visible`。修复后的单 vCPU Venus VM 能通过 SSH 验收，内核记录 `Host memory window: 0xe0000000 +0x10000000` 和 `+host_visible`。

| 最终构建的实测 | 结果 | 就绪耗时 | 证据目录（相对 `.build/windows`） |
|---|---|---|---|
| 单 vCPU software | SSH、virtio_gpu、DRM 节点通过 | 34.21 秒 | `linux-software-final` |
| 单 vCPU Venus | SSH、virtio_gpu、256MB host-visible 通过 | 首次未记录耗时 | `linux-venus-single` |
| 单 vCPU Venus 重复启动 | 同上，宿主 AMD Radeon 780M Graphics | 32.62 秒 | `linux-venus-repeat` |

这些耗时包含 VM 启动和首次 cloud-init 配置，仅用于启动就绪记录，不作为 GPU 性能结果。三次均正常清理本次 VM，最后两份完整报告记录了同一个 VBoxDD SHA256。

修复遵循 [Linux PCI 能力定义](https://github.com/torvalds/linux/blob/v6.8/include/uapi/linux/virtio_pci.h) 和 [VirtIO-GPU 区域 ID 定义](https://github.com/torvalds/linux/blob/v6.8/include/uapi/linux/virtio_gpu.h)：

- 共享内存能力必须为 24 字节；ID 位于公共头偏移 5，GPU 的 host-visible ID 为 1。
- MSI-X 必须接在共享能力之后，注册失败时终止链表。
- GPU 保存状态版本改为 9，拒绝包含旧 PCI 能力布局的版本 8 快照。关机后的 VM 配置和磁盘不受此限制。

新增回归调用生产 PCI 构造函数并遍历原始配置字节，覆盖共享 BAR 开/关、MSI-X 开/关和注册失败；修复前复现 7 个错误。修复后包含此项的 42 组设备测试及 VBoxDD 注册入口检查通过。

## 已知边界

- BIOS 仍报告 BAR4 的初始分配失败，但本次 Linux 成功将其重新分配到 `0xe0000000`，并接受完整 256MB 窗口。其他固件/客户机组合尚未验收。
- 双 vCPU 配置曾在早期初始化阶段停顿并超时；单 vCPU 配置已通过。尚未确认多核停顿的根因，也未进行长时间稳定性验证。
- 当前 EDID 存在校验告警，驱动报告没有可用的 CRTC/尺寸；桌面显示、分辨率切换仍未通过。
- 完整 Mesa Venus 初始化、客户机 Vulkan device、图形/计算工作负载和性能对照仍待实现与验收。

启动观察脚本另有 `tools/test-vm-observation.ps1`，在不启动真实 VM 的情况下验证正常观察、中途 aborted、观察末尾关机、状态缺失和清理失败五个场景。
