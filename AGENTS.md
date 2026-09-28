# AGENTS.md - virtualbox-virtio-gpu-venus handoff

更新日期：2026-09-28

## 项目目标

在 Windows 宿主上编译 VirtualBox，并通过 VirtIO-GPU/Venus 将宿主机 Vulkan GPU 能力提供给 Linux 客体。方案不使用 PCIe 直通，依赖 VirtIO-GPU、共享显存 BAR 和 Vulkan 命令转发。

## 当前仓库状态

- 工作目录：`D:\code\virtualbox-virtio-gpu-venus`
- 分支：`main`
- 远端：`git@github.com:wso4133560/virtualbox-virtio-gpu-venus.git`
- 本轮源码验证基于工作树最终 DLL `FE0A10401151BA094A6D78877134D9D204E00B7FFC2190DE7B011D4711DFF7BD`；提交哈希以 `git HEAD` 和远端分支为准。
- 前两个相关提交：`e239d8c9`（Replay recorded Venus command buffers on submit）、`81dc9bf9`（Package current Venus guest evidence）；更早提交 `2976c1c5`（ring wrap-around/reply bounds）、`72421ab1`（zero output handles in Venus creates）。
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

结果：当前报告 `.build\windows\virtio-gpu-validation.json` 退出码 `0`，52 个测试组通过、`missingGroups=[]`。回归包含 blob resource/Vulkan backing、ring metadata/reply/progress、multi-stream framing/reply cursor、ring 尾部 wrap-around 与 reply 越界、Vulkan transfer/clear/barrier/blit/fill/update、binary/timeline semaphore query/signal/wait、host fence/semaphore 生命周期、classic/submit2 wait-signal、对象依赖顺序 save/load，以及从构建产物注册 VBoxDD 的检查。

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
- 当前构建双 vCPU 重试报告 `.build\windows\linux-venus-cursor-final-cpu2-long\report.json`：`cpuCount=2`、`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 为 `3CA41E3DEB841CA85B04B62B8FD272777C62365253410F660532E1B00BFAB411`。该报告覆盖当前构建的双 vCPU smoke，不替代长时间稳定性矩阵。
- `tools/test-linux-virtio-gpu.ps1 -WorkloadSeconds N` 现在将远端 workload 超时设置为 `max(30, N + 30)` 秒；修复前固定 30 秒会把 60 秒 workload 误报为退出码 124。脚本修复后的单 vCPU 5 秒报告 `.build\windows\linux-venus-final-current5-scriptfix\report.json` 通过，3009 次迭代、`cleanupErrors=[]`。本轮双 vCPU 60 秒尝试未进入 SSH/Vulkan：Ubuntu 在 `raid6` 模块初始化阶段超过 420 秒，不能作为 Venus 协议失败或通过证据。
- reset 报告 `.build\windows\linux-venus-reset-admin51\report.json`：`resetVerified=true`、`resetVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；该次使用 `-SkipGuestVulkanInfo`，只把 reset 前后的 Vulkan buffer/fill/readback workload 作为重连证据，完整 `vulkaninfo` 由 admin46 报告覆盖。
- 当前对象生命周期改动后的标准报告 `.build\windows\linux-venus-object-admin52\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 本阶段最终对象/录制语义标准报告 `.build\windows\linux-venus-opaque-objects5\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=3181`、`guestVulkanWorkloadExit=0`、`cleanupErrors=[]`、`passed=true`；实际 `VBoxDD.dll` SHA256 为 `98EB31165B57DF0406A23AB0A5A323719EA5AC948ED0202834BC469C35FD0F92`。
- 本阶段最终 30 秒压力报告 `.build\windows\linux-venus-recording-defer30\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=21459`、`guestVulkanWorkloadExit=0`、`cleanupErrors=[]`、`passed=true`；实际 `VBoxDD.dll` SHA256 为 `6479C99D37AC656CE7E61A9A58D0B3C5E6855157DDADB0238DFCD88297C871AF`，VBox.log 无 `ring command rejected`。
- 本阶段最终 saved-state 报告 `.build\windows\linux-venus-recording-save5\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；恢复 workload 3096 次迭代，恢复日志包含 `VULKANINFO_PASS` 和 `VULKAN_WORKLOAD_PASS`。
- 本阶段最终 reset 报告 `.build\windows\linux-venus-recording-reset5\report.json`：`resetVerified=true`、`resetVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；reset 后 workload 3221 次迭代，日志包含 `VULKANINFO_PASS` 和 `VULKAN_WORKLOAD_PASS`。
- 当前对象生命周期改动后的 saved-state 报告 `.build\windows\linux-venus-object-save-admin53\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；恢复日志包含 `VULKANINFO_PASS` 和 `VULKAN_WORKLOAD_PASS`。
- 当前对象生命周期改动后的 reset 报告 `.build\windows\linux-venus-object-reset-admin55\report.json`：`resetVerified=true`、`resetVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 当前 `vkQueueSubmit2`/idle 回复改动后的标准报告 `.build\windows\linux-venus-submit2-admin56\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 当前 semaphore/framing 改动后的标准报告 `.build\windows\linux-venus-semaphore\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；运行时哈希指向同一目录生成的 `VBoxDD.dll`。
- 当前 semaphore/framing 改动的 saved-state restore workload 已在 `.build\windows\linux-venus-semaphore-full\guest-vulkan-restore.log` 中看到 `VULKANINFO_PASS`、`VULKAN_WORKLOAD_PASS` 和退出码 0；同一组合脚本在 reset 后 SSH 重连阶段被中止，未生成最终 `report.json`，reset 结论沿用已通过的 `linux-venus-object-reset-admin55` 报告。
- 当前 timeline semaphore 改动后的标准报告 `.build\windows\linux-venus-timeline\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；runtime hash 指向当前构建的 `VBoxDD.dll`。
- 最终边界修复后的标准报告 `.build\windows\linux-venus-timeline-final\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；`VBoxDD.dll` SHA256 为 `D0DCB3ECE55F71B861371B58A6BB29A8FC1C81C9CCCA8847CFAB499BA24178BC`。
- 当前 timeline semaphore 改动后的 saved-state 报告 `.build\windows\linux-venus-timeline-save\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；恢复日志同时包含 `VULKANINFO_PASS`、`VULKAN_WORKLOAD_PASS` 和退出码 0。
- 当前对象分发和录制语义改动后的宿主回归 `.build\windows\virtio-gpu-validation.json`：最新无压力回归 51 组通过，包含 shader/pipeline layout/sampler/descriptor set layout/descriptor pool opaque object 生命周期、录制阶段延迟执行、多 stream descriptor/reply position、跨 ring 尾部命令读取和 reply seek 边界检查。
- 当前宿主同步对象改动后的回归 `.build\windows\virtio-gpu-validation.json`：52 组通过，新增真实 host fence 创建/查询/重置/销毁与 binary/timeline semaphore host handle 检查，并验证 reset 清理对象表；最终 `VBoxDD.dll` SHA256 为 `FE0A10401151BA094A6D78877134D9D204E00B7FFC2190DE7B011D4711DFF7BD`。
- 当前宿主同步对象改动后的客体标准报告 `.build\windows\linux-venus-host-sync3\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=3288`、`guestVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 当前宿主同步对象改动后的客体重复启动报告 `.build\windows\linux-venus-host-sync-repeat3\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=3516`、`guestVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 当前宿主同步对象改动后的 saved-state 报告 `.build\windows\linux-venus-host-sync-save3\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、首次 workload 3027 次、恢复 workload 3073 次、`cleanupErrors=[]`、`passed=true`。
- 当前宿主同步对象改动后的 reset 报告 `.build\windows\linux-venus-host-sync-reset3\report.json`：`resetVerified=true`、`resetVulkanExit=0`、首次 workload 3267 次、reset workload 3141 次、`cleanupErrors=[]`、`passed=true`。
- 当前 host Vulkan object 依赖顺序修复后的客体标准报告 `.build\windows\linux-venus-host-objects7\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=3268`、`guestVulkanWorkloadExit=0`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 为 `B56CA6211B7C6DC5D602C6F4D1558BC33CCF8065B73976EFA95DA061B3CE9DE7`。
- 当前 host Vulkan object 依赖顺序修复后的 30 秒客体压力报告 `.build\windows\linux-venus-host-objects30-7\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=21195`、`guestVulkanWorkloadExit=0`、`cleanupErrors=[]`、`passed=true`；日志无 `ring command rejected`、`stuck` 或 `expired ring`。
- 当前 host Vulkan object 依赖顺序修复后的独立 saved-state 报告 `.build\windows\linux-venus-host-objects-save7\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`cleanupErrors=[]`、`passed=true`；首次/恢复 workload 分别 2924/3073 次，两个日志均包含 `VULKANINFO_PASS` 与 `VULKAN_WORKLOAD_PASS`。
- 当前 host Vulkan object 依赖顺序修复后的独立 reset 报告 `.build\windows\linux-venus-host-objects-reset7\report.json`：`resetVerified=true`、`resetVulkanExit=0`、`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`cleanupErrors=[]`、`passed=true`；首次/reset workload 分别 3087/3407 次，两个日志均包含 `VULKANINFO_PASS` 与 `VULKAN_WORKLOAD_PASS`。
- 当前对象 ID dispatcher 改动后的客体标准报告 `.build\windows\linux-venus-final-object-id\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 为 `E0FF994AC0AFCFB2BAD4B74DA4790CEFB03A716714D37F219BF63AA45F24A157`，日志无 `ring command rejected`。
- 当前对象 ID dispatcher 改动后的客体重复启动报告 `.build\windows\linux-venus-final-object-id-repeat\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 同为 `E0FF994AC0AFCFB2BAD4B74DA4790CEFB03A716714D37F219BF63AA45F24A157`。
- 当前构建的客体 saved-state 报告 `.build\windows\linux-venus-final-object-id-save\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`cleanupErrors=[]`、`passed=true`；第一次和恢复后的日志均包含 `VULKANINFO_PASS`、`VULKAN_WORKLOAD_PASS`，且无 ring rejection。
- 当前 reply cursor/multi-stream 修复后的客体标准报告 `.build\windows\linux-venus-cursor-final\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 为 `3CA41E3DEB841CA85B04B62B8FD272777C62365253410F660532E1B00BFAB411`，日志无 `ring command rejected`。
- 当前 reply cursor/multi-stream 修复后的客体重复启动报告 `.build\windows\linux-venus-cursor-final-repeat\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 同为 `3CA41E3DEB841CA85B04B62B8FD272777C62365253410F660532E1B00BFAB411`。
- 当前 reply cursor/multi-stream 修复后的客体 saved-state 报告 `.build\windows\linux-venus-cursor-final-save\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`cleanupErrors=[]`、`passed=true`；首次和恢复日志均包含 `VULKANINFO_PASS`、`VULKAN_WORKLOAD_PASS`，且无 ring rejection。
- 当前 reply cursor/multi-stream 修复后的客体 reset 报告 `.build\windows\linux-venus-cursor-final-reset\report.json`：`resetVerified=true`、`resetVulkanExit=0`、`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 同为 `3CA41E3DEB841CA85B04B62B8FD272777C62365253410F660532E1B00BFAB411`。
- 当前最终运行时包 `.build\windows\virtualbox-virtio-gpu-venus-final.zip`：257 个文件，ZIP SHA256 为 `D02FB807AAB6A815826C44CFC5CB57266417C856FB30DFC8DEA1661F1606C41A`；包内包含宿主 52 组回归、host-sync 标准/重复启动/saved-state/reset 报告和最终 DLL 客体证据。包校验脚本报告 manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。
- 当前命令缓冲区重放修复后的客体压力报告 `.build\windows\linux-venus-command-buffer-replay30\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=21266`、`workloadSeconds=30`、`cleanupErrors=[]`、`passed=true`。该报告的 `VBoxDD.dll` SHA256 为 `BBA10F4168EAB68C616DD61F3BA81650BB3248F39946C84E15744B92BE1A7EF1`。
- 当前最终 DLL 的客体报告 `.build\windows\linux-venus-final-current5\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=3001`、`workloadSeconds=5`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；`VBoxDD.dll` SHA256 为 `4A785BEA01F06DA03CEB8BDD84FDB963E881E25F072E8B73F881DF4F3F419040`。
- 同一命令缓冲区重放实现的 saved-state 与 reset 报告分别为 `.build\windows\linux-venus-command-buffer-replay-save5\report.json`（`saveRestoreVerified=true`、恢复后 Vulkan exit 0）和 `.build\windows\linux-venus-command-buffer-replay-reset5\report.json`（`resetVerified=true`、reset 后 Vulkan exit 0），两者均 `cleanupErrors=[]`、`passed=true`。这两份报告对应的中间 DLL SHA256 为 `BBA10F4168EAB68C616DD61F3BA81650BB3248F39946C84E15744B92BE1A7EF1`；最终 DLL 已用 `linux-venus-final-current5` 复验。
- `tools\test-linux-virtio-gpu.ps1` 的清理路径现在会在 VBoxHeadless 退出期间短暂重试失效 direct-session 查询；若 runtime 进程已退出则按停止处理，仍保留对存活进程和真实注销失败的报错。
- 当前 timeline semaphore 改动后的独立 reset 报告 `.build\windows\linux-venus-timeline-reset2\report.json`：`resetVerified=true`、`resetVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；reset workload 日志包含 `VULKAN_WORKLOAD_PASS`。
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
10. `vkSeekReplyCommandStreamMESA`（命令类型 `179`）校验并更新 reply cursor；reply 控制命令按当前 cursor 写入共享 reply stream 并推进 4-byte reply slot，拒绝无效 stream/cursor。
11. `vkExecuteCommandStreamsMESA` 从共享 blob 读取 descriptor/command stream，处理多 stream、显式 reply position 和嵌套 ring 命令，并执行当前支持的 transfer/clear/barrier/blit/fill/update 子集。
12. ring reply cursor、reply validity、buffer/memory binding、有限 command buffer/fence/binary/timeline semaphore 生命周期状态、classic/`vkQueueSubmit2` wait-signal、`vkGetSemaphoreCounterValue`/`vkWaitSemaphores`/`vkSignalSemaphore`、idle 回复，以及 saved-state version `20` 的 save/load 和一致性检查。
13. 对已支持的 transfer/barrier command stream 保存有界命令字节，在 `vkQueueSubmit`/`vkQueueSubmit2` 中按 command-buffer 顺序重放；Begin/Reset/Free 清理旧记录，saved-state 持久化命令流，修复 Mesa fence feedback slot 在 reset 后未被重新写入的问题。
14. image object handle 到已绑定 host-visible resource 的统一解析；环命令与 `SUBMIT_3D` 的 image/buffer transfer、clear、barrier、copy、blit 路径不再强制截断 64 位 Vulkan handle，并保留旧 resource ID 回退。
15. `vkGetDeviceMemoryCommitment`、`vkGetImageMemoryRequirements`、`vkGetImageMemoryRequirements2`、`vkBindImageMemory`、`vkBindImageMemory2` 的有界 framing/reply 和 binding-table 状态更新。
16. 有界 opaque Vulkan object table 已覆盖 `vkCreate/DestroyShaderModule`、`vkCreate/DestroyPipelineLayout`、`vkCreate/DestroySampler`、`vkCreate/DestroyDescriptorSetLayout` 和 `vkCreate/DestroyDescriptorPool`；后四类已实际创建/销毁宿主 Vulkan 对象，create wire 有界保存并在 saved-state 恢复时重建，创建参数、动态数组、句柄输出、销毁类型匹配和 saved-state 类型校验均已接入。
17. command buffer 录制阶段只保存已校验的 transfer/barrier/fill 命令，资源执行延迟到 queue submit/replay；这避免异步 memory bind 尚未可见时把合法录制命令误报为 ring fatal。

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

1. 继续补齐通用 Venus Vulkan object/query/reply dispatcher，尤其是更多 physical-device/device/query、同步和句柄映射路径；当前 command buffer/fence/semaphore 状态仍是有界生命周期表，不等于完整驱动对象后端。
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

## 2026-09-28 本轮最终证据

- 提交 `1efdc39a450984bd1c1b46495bec7da09a77b28c` 已推送到 `origin/main`。
- 宿主回归 `.build\\windows\\virtio-gpu-validation.json`：55 组通过，包含 physical image format、external buffer/fence/semaphore、descriptor set layout support、host-backed image layout/memory requirements；`-IncludeRegistration` 通过。
- 客体标准报告 `.build\\windows\\linux-venus-image-next\\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、20 秒 workload 14142 次、`vboxSupState=RUNNING`、`passed=true`。
- 客体 saved-state 报告 `.build\\windows\\linux-venus-image-save4\\report.json`：`saveRestoreVerified=true`、恢复 Vulkan exit 0、初始/恢复日志均含 `VULKANINFO_PASS` 和 `VULKAN_WORKLOAD_PASS`、`passed=true`。
- 客体 reset 报告 `.build\\windows\\linux-venus-image-reset3\\report.json`：`resetVerified=true`、reset Vulkan exit 0、`passed=true`；reset 前增加 `vkDeviceWaitIdle` 后不再出现 `VBoxHeadless` 访问冲突。
- 最终开发包 `.build\\windows\\virtualbox-virtio-gpu-venus-image-final.zip`：257 个文件，ZIP SHA256 `463CEF63F24CDBDC4C7EB8D5139E226E2B25F14102D04C00E2CB6958280E88F1`；包内外 `VBoxDD.dll` SHA256 `1D564863030A408124E348457F23708D5D9F9DD1081502755C24B625D6F523C6`，包校验通过。

## 2026-09-28 image-view 追加验收

- `DevVirtioGPU.cpp` 新增 host-backed `vkCreateImageView`/`vkDestroyImageView`（命令 57/58），校验 `VkImageViewCreateInfo` 的 pNext、allocator、subresource range 和 image 依赖；析构时先释放 image view 再释放 image。
- saved-state opaque object 校验与重建顺序已包含 image -> image view -> pipeline layout；宿主回归新增 `Venus image view lifecycle`，共 56 组通过，包含注册检查。
- 客体报告 `.build\\windows\\linux-venus-image-view-final\\report.json`：标准 `vulkaninfo`、15 秒 workload（10220 次）和 saved-state restore 均通过，`vboxSupState=RUNNING`、`cleanupErrors=[]`；同一报告的 save 后紧接 reset 因 SSH 未恢复而记录 `resetVerified=false`，不能作为 reset 通过证据。
- 独立 fresh-VM reset 报告 `.build\\windows\\linux-venus-image-view-reset\\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`resetVerified=true`、`resetVulkanExit=0`、`passed=true`。
- 当前开发包 `.build\\windows\\virtualbox-virtio-gpu-venus-image-view-final.zip`：257 个文件，ZIP SHA256 `42162B323B86EB28D2D8335623CCB569A4A9BB5BC820C72AF9F993A0B2B92468`；包内外 `VBoxDD.dll` SHA256 `7BF5A6D6E3B223775DE7BBD1658F6760A61EE5BBB6CC975B06C1257CD3E96C51`，manifest、版本、VirtIO-GPU/Venus、PE/VMMR0 四项校验通过。

只提交本阶段相关文件，保留测试报告在 `.build` 下，不要提交临时 VM 密钥、磁盘或 core dump。
