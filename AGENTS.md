# AGENTS.md - virtualbox-virtio-gpu-venus handoff

更新日期：2026-09-28

## 项目目标

在 Windows 宿主上编译 VirtualBox，并通过 VirtIO-GPU/Venus 将宿主机 Vulkan GPU 能力提供给 Linux 客体。方案不使用 PCIe 直通，依赖 VirtIO-GPU、共享显存 BAR 和 Vulkan 命令转发。

## 当前仓库状态

- 工作目录：`D:\code\virtualbox-virtio-gpu-venus`
- 分支：`main`
- 远端：`git@github.com:wso4133560/virtualbox-virtio-gpu-venus.git`
- 最新已推送提交：`dc642a606db9f1873d774b5b4d45a5f0ce22cbf2`（Venus queue submit2 lifecycle）
- 前两个相关提交：`df51bd21`（ring notify/command-stream execution）、`0116a7f5`（ring protocol state/progress）
- 开始新工作先执行 `git status --short`；交接时应保持工作树干净。

## Windows 编译环境

已验证环境：Visual Studio 2022 Community、Windows SDK/WDK `10.0.26100.0`、amd64、PowerShell。

仓库生成配置位于：

- `.build\windows\AutoConfig.kmk`
- `.build\windows\LocalConfig.kmk`
- `.build\windows\env.bat`

直接增量编译 VBoxDD 和回归测试：

```powershell
cmd /d /s /c 'call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 && call "D:\code\virtualbox-virtio-gpu-venus\.build\windows\env.bat" && cd /d "D:\code\virtualbox-virtio-gpu-venus\VirtualBox-7.2.6" && kmk AUTOCFG="D:\code\virtualbox-virtio-gpu-venus\.build\windows\AutoConfig.kmk" LOCALCFG="D:\code\virtualbox-virtio-gpu-venus\.build\windows\LocalConfig.kmk" SDK_WINSDK10_MAX_VERSION=10.0.26100.0 VBOX_WITH_VIRTIO_GPU=1 VBOX_WITH_VIRTIO_GPU_VENUS=1 -j1 VBoxDD tstVirtioGPU'
```

`tools/build-windows.ps1` 是完整构建入口，但当前 checkout 缺少部分可选源码目录（例如 libssh/keygen），完整目标可能被这些依赖阻塞。上面的增量目标可以生成 `DevVirtioGPU.cpp`、链接 `VBoxDD.dll` 并生成 `tstVirtioGPU.exe`；Qt6 缺失提示目前是 warning。

已知 kBuild 限制：在当前 Windows 环境中，增量命令可能在编译已经完成后报：

```text
kmk: Failed to create stdout pipe: 5
kmk: Failed to create worker threads
```

遇到该错误时，不要将其误判为 C/C++ 编译错误。可从以下生成的 `.dep` 文件提取并单独执行 `cl`/`link` 行，完成本阶段所需的四个产物：

- `VirtualBox-7.2.6\out\win.amd64\release\obj\VBoxDD\VirtIO\DevVirtioGPU.obj.dep`
- `VirtualBox-7.2.6\out\win.amd64\release\obj\tstVirtioGPU\tstVirtioGPU.obj.dep`
- `VirtualBox-7.2.6\out\win.amd64\release\obj\VBoxDD\VBoxDD.dep`
- `VirtualBox-7.2.6\out\win.amd64\release\obj\tstVirtioGPU\tstVirtioGPU.dep`

## 已验证结果

### Windows 宿主回归

最近一次宿主验证命令：

```powershell
.\tools\test-virtio-gpu.ps1 -TimeoutSeconds 60 -IncludeRegistration
```

结果：退出码 `0`，报告 `.build\windows\virtio-gpu-validation.json`，共 `43` 个测试组通过，`missingGroups=[]`。已包含 blob resource/Vulkan backing、ring metadata/reply/progress、Vulkan transfer/clear/barrier/blit/fill/update、save/load、以及从构建产物注册 VBoxDD 的检查。

这表示 Windows 用户态传输、设备回调和宿主 Vulkan 路径通过了当前回归边界。

### Windows 宿主 Vulkan/GPU

```powershell
.\tools\test-windows-gpu-utilization.ps1
```

已验证宿主设备为 AMD Radeon 780M Graphics，Vulkan `1.4.349`；历史报告为 `.build\windows\gpu-utilization-validation.json`。这只证明宿主 Vulkan loader/device/queue 和利用率路径，不证明客体 Vulkan。

### Linux 客体

标准命令：

```powershell
.\tools\test-linux-virtio-gpu.ps1 `
  -ImagePath '.build\images\noble-20260911-amd64.img' `
  -GpuBackend venus `
  -VerifyGuestVulkan `
  -ReportDirectory '.build\windows\linux-venus-next'
```

镜像必须匹配 `tools\linux-test-image.json` 中的 pinned bytes/SHA256。脚本会创建临时 VM、SSH 转发和报告目录，并在结束时清理 VM。

当前客体验证边界：

- 最终标准报告 `.build\windows\linux-venus-final-admin46\report.json`：`guestReady=true`、`sshReady=true`、`drmDriverBound=true`、`hostVisible=true`、`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`cleanupErrors=[]`、`passed=true`；`guest-vulkan.log` 中 `vulkaninfo_exit=0`、`VULKANINFO_PASS` 和 `VULKAN_WORKLOAD_PASS` 均存在。
- saved-state 报告 `.build\windows\linux-venus-save-restore-admin47\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；恢复日志同时包含 `VULKANINFO_PASS` 和 `VULKAN_WORKLOAD_PASS`。
- 双 vCPU 报告 `.build\windows\linux-venus-cpu2-admin48\report.json`：`cpuCount=2`、`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`cleanupErrors=[]`、`passed=true`；这是一轮启动与 smoke 验证，不替代长时间压力测试。
- reset 报告 `.build\windows\linux-venus-reset-admin51\report.json`：`resetVerified=true`、`resetVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；该次使用 `-SkipGuestVulkanInfo`，只把 reset 前后的 Vulkan buffer/fill/readback workload 作为重连证据，完整 `vulkaninfo` 由 admin46 报告覆盖。
- 当前对象生命周期改动后的标准报告 `.build\windows\linux-venus-object-admin52\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 当前对象生命周期改动后的 saved-state 报告 `.build\windows\linux-venus-object-save-admin53\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；恢复日志包含 `VULKANINFO_PASS` 和 `VULKAN_WORKLOAD_PASS`。
- 当前对象生命周期改动后的 reset 报告 `.build\windows\linux-venus-object-reset-admin55\report.json`：`resetVerified=true`、`resetVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 当前 `vkQueueSubmit2`/idle 回复改动后的标准报告 `.build\windows\linux-venus-submit2-admin56\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 旧的 `linux-venus-final-ring`、`linux-venus-binding-admin41` 等失败报告保留为历史诊断证据；它们不代表当前 runtime 的最终状态。

客体重试时必须使用同一 runtime 目录下的 `VBoxManage.exe`、`VBoxSVC.exe`、`VBoxC.dll`、`VBoxDD.dll`，并使用独立的 `VBOX_USER_HOME`；不要混用系统安装版 VirtualBox。

## 当前已完成的实现

1. Windows VirtIO-GPU 设备、PCI shared-memory capability、host-visible BAR。
2. 宿主 Vulkan loader/device/queue 初始化和资源同步。
3. HOST3D blob 创建、映射/取消映射、Vulkan buffer/image backing。
4. 多个 Vulkan transfer、clear、barrier、blit、fill、update 命令及批处理。
5. `CTX_CREATE` 兼容 Venus capset id `4`，同时保留旧测试使用的 init 值 `1`。
6. `RESOURCE_MAP_BLOB`/`UNMAP_BLOB` 按 common header 已消费后只读取 8-byte body。
7. ring 元数据、ring/virtqueue seqno 控制命令识别，以及 BO-only submit 的共享 ring head/status 推进。
8. `vkCreateRingMESA` 的 ring 元数据持久化，`vkDestroyRingMESA`、`vkNotifyRingMESA`、`vkWriteRingExtraMESA`、`vkSubmitVirtqueueSeqnoMESA`、`vkWait*SeqnoMESA` 的当前状态处理。
9. `vkSetReplyCommandStreamMESA` 保存 reply resource/offset/size，并初始化 reply cursor。
10. `vkSeekReplyCommandStreamMESA`（命令类型 `179`）校验并更新 reply cursor；reply 控制命令按当前 cursor 写入共享 reply stream。
11. `vkExecuteCommandStreamsMESA` 从共享 blob 读取 descriptor/command stream，处理嵌套 ring 命令，并执行当前支持的 transfer/clear/barrier/blit/fill/update 子集。
12. ring reply cursor、reply validity、buffer/memory binding、有限 command buffer/fence 生命周期状态、`vkQueueSubmit2`/idle 回复，以及 saved-state version `15` 的 save/load 和一致性检查。

当前宿主侧主要路径：

```text
Mesa vn_*_MESA submit/call
  -> virtioGpuR3ProcessVenusRings
  -> virtioGpuR3HandleVenusRingCommand
  -> virtioGpuR3ExecuteVenusCommandStream
  -> Vulkan decode/execute helpers
  -> virtioGpuR3RingWriteReply / shared head-status publish
```

## 未完成项和验证边界

完整 Venus renderer protocol 仍未完成，但当前实现已通过 Linux 客体 `vulkaninfo --summary`、host-visible Vulkan fill/readback workload 和 saved-state restore workload。下一阶段按以下顺序推进：

1. 补齐通用 Venus Vulkan object/query/reply dispatcher，尤其是更多 physical-device/device/query、同步和句柄映射路径；当前 command buffer/fence 状态仍是有界生命周期表，不等于完整驱动对象后端。
2. 完善多命令 stream framing、reply payload、返回值和错误码，持续避免仅推进 head/tail/status 的 bring-up 语义。
3. 对 `vkNotifyRingMESA`、`vkWriteRingExtraMESA`、`vkSubmitVirtqueueSeqnoMESA`、`vkWait*SeqnoMESA` 增加更广泛的 Mesa 版本边界和异常测试。
4. 扩展真实客户机图形 workload、长时间压力、多 vCPU、桌面显示和性能矩阵；当前标准 smoke 与 saved-state restore 已通过。

当前 head/tail/status 推进包含 bring-up 逻辑，可防止宿主立即死等，但不等于命令已经完整在宿主 GPU 上执行。

## 调试方法和源码位置

- 客体 Vulkan 调试脚本设置 `VN_DEBUG=init` 和 `VK_LOADER_DEBUG=all`。
- 客体日志：报告目录中的 `guest-vulkan.log`。
- 宿主 VM 日志：报告目录中的 `VBox.log`；串口日志：`serial.log`。
- 重点搜索：

```powershell
rg -n 'stuck|failed|MESA-VIRTIO|SUBMIT_3D|virtio-gpu:' .build\windows\linux-venus-*\guest-vulkan.log .build\windows\linux-venus-*\VBox.log
```

- Mesa 协议定义：`VirtualBox-7.2.6\src\VBox\Additions\3D\mesa\mesa-24.0.2\src\virtio\venus-protocol\vn_protocol_driver_transport.h`
- Mesa ring 实现：`VirtualBox-7.2.6\src\VBox\Additions\3D\mesa\mesa-24.0.2\src\virtio\vulkan\vn_ring.c`
- 宿主实现：`VirtualBox-7.2.6\src\VBox\Devices\VirtIO\DevVirtioGPU.cpp`
- 当前 Mesa `vn_ring_get_layout()` 的默认布局是 head offset `0`、tail offset `64`、status offset `128`，均按 64-byte 对齐；这是当前版本的事实，协议实现仍应优先使用 `vkCreateRingMESA` 传入的字段。

## Windows COM/驱动问题

`tools\register-windows-runtime.ps1` 成功只表示 runtime COM 注册验证通过。若 `VBoxManage.exe list hostinfo` 报 `ERROR_FILE_NOT_FOUND`，先确认使用同一 runtime 目录下的 `VBoxManage.exe`、`VBoxSVC.exe`、`VBoxC.dll` 和 `VBoxDD.dll`，不要混用系统安装版 VirtualBox。

`sc.exe query VBoxSup` 返回 `1060` 表示 VBoxSup 服务未安装。驱动测试签名、VBoxSup/VBoxSup-inf 安装和管理员权限属于独立阶段；不要把 COM 注册成功当作驱动已安装。

## Git 交接

用户已授权阶段性提交和 push。提交前执行：

```powershell
git diff --check
git status --short
git add <相关文件>
git commit -m "<message>"
git push origin main
git rev-parse HEAD
git ls-remote origin refs/heads/main
```

只提交本阶段相关文件，保留测试报告在 `.build` 下，不要提交临时 VM 密钥、磁盘或 core dump。
