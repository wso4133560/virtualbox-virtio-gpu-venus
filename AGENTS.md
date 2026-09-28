# AGENTS.md - virtualbox-virtio-gpu-venus handoff

更新日期：2026-09-29

## 项目目标

在 Windows 宿主上编译 VirtualBox，并通过 VirtIO-GPU/Venus 将宿主机 Vulkan GPU 能力提供给 Linux 客体。方案不使用 PCIe 直通，依赖 VirtIO-GPU、共享显存 BAR 和 Vulkan 命令转发。

## 当前仓库状态

- 工作目录：`D:\code\virtualbox-virtio-gpu-venus`
- 分支：`main`
- 远端：`git@github.com:wso4133560/virtualbox-virtio-gpu-venus.git`
- 本轮源码验证基于工作树最终 DLL `7CEF43BE69538014B4565304F85FBC57E5B47608C742A3FA907D999887227CB7`；提交哈希以 `git HEAD` 和远端分支为准。
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

结果：当前报告 `.build\windows\virtio-gpu-validation.json` 退出码 `0`，59 个测试组通过、`missingGroups=[]`。回归包含 blob resource/Vulkan backing、ring metadata/reply/progress、multi-stream framing/reply cursor、ring 尾部 wrap-around 与 reply 越界、Vulkan transfer/clear/barrier/blit/fill/update、binary/timeline semaphore/query pool query/signal/wait、host fence/semaphore/event 生命周期、classic/submit2 wait-signal、对象依赖顺序 save/load，以及从构建产物注册 VBoxDD 的检查。

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
- 当前事件生命周期改动后的客体标准报告 `.build\windows\linux-venus-event-final-long\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`guestVulkanWorkloadIterations=2911`、`guestVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 为 `C443444D3AFA0B5FB9D5A9C428DA057AAA375E3709B720A5A527203A6AE43BDB`。
- 当前事件生命周期改动后的 saved-state 报告 `.build\windows\linux-venus-event-save5\report.json`：`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、首次 workload 3017 次、`guestVulkanVerified=true`、`cleanupErrors=[]`、`passed=true`；事件逻辑状态随 saved-state 保存，版本为 21。
- 当前事件生命周期改动后的 reset 报告 `.build\windows\linux-venus-event-reset5\report.json`：`resetVerified=true`、`resetVulkanExit=0`、首次 workload 3245 次、`guestVulkanVerified=true`、`cleanupErrors=[]`、`passed=true`。

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
12. ring reply cursor、reply validity、buffer/memory binding、有限 command buffer/fence/binary/timeline semaphore/query pool/descriptor set 生命周期状态、classic/`vkQueueSubmit2` wait-signal、`vkGetSemaphoreCounterValue`/`vkWaitSemaphores`/`vkSignalSemaphore`、idle 回复，以及 saved-state version `24` 的 save/load 和一致性检查。
13. 对已支持的 transfer/barrier command stream 保存有界命令字节，在 `vkQueueSubmit`/`vkQueueSubmit2` 中按 command-buffer 顺序重放；Begin/Reset/Free 清理旧记录，saved-state 持久化命令流，修复 Mesa fence feedback slot 在 reset 后未被重新写入的问题。
14. image object handle 到已绑定 host-visible resource 的统一解析；环命令与 `SUBMIT_3D` 的 image/buffer transfer、clear、barrier、copy、blit 路径不再强制截断 64 位 Vulkan handle，并保留旧 resource ID 回退。
15. `vkGetDeviceMemoryCommitment`、`vkGetImageMemoryRequirements`、`vkGetImageMemoryRequirements2`、`vkBindImageMemory`、`vkBindImageMemory2` 的有界 framing/reply 和 binding-table 状态更新。
16. 有界 opaque Vulkan object table 已覆盖 `vkCreate/DestroyShaderModule`、`vkCreate/DestroyPipelineLayout`、`vkCreate/DestroySampler`、`vkCreate/DestroyDescriptorSetLayout` 和 `vkCreate/DestroyDescriptorPool`；后四类已实际创建/销毁宿主 Vulkan 对象，create wire 有界保存并在 saved-state 恢复时重建，创建参数、动态数组、句柄输出、销毁类型匹配和 saved-state 类型校验均已接入。
17. command buffer 录制阶段只保存已校验的 transfer/barrier/fill 命令，资源执行延迟到 queue submit/replay；这避免异步 memory bind 尚未可见时把合法录制命令误报为 ring fatal。
18. `vkCreateEvent`/`vkDestroyEvent`/`vkGetEventStatus`/`vkSetEvent`/`vkResetEvent`（命令 42-46）已接入有界 wire framing、真实宿主 `VkEvent` 生命周期和状态回复；事件逻辑状态与 create wire 纳入 saved-state version 21，恢复时重建并重新设置宿主事件。
19. `vkCreateQueryPool`/`vkDestroyQueryPool`/`vkGetQueryPoolResults`/`vkResetQueryPool`（命令 47-49、171）已接入动态 command-size 校验、真实宿主 `VkQueryPool` 创建/销毁/重置和 `VkResult + array-size + blob` 结果回复；query pool create wire 纳入 saved-state version 22，恢复时按独立 pass 重建宿主句柄。
20. `vkAllocateDescriptorSets`/`vkFreeDescriptorSets`/`vkUpdateDescriptorSets`（命令 77-79）已接入严格的单 layout/单输出 descriptor set framing、真实宿主 `VkDescriptorSet` 分配/释放、动态 write/copy framing，以及常见 sampler/image/buffer descriptor 到宿主 `vkUpdateDescriptorSets` 的解码；未知 pNext、texel buffer、数组不匹配和错误 pool 仍拒绝，descriptor update wire 已纳入 saved-state 并在依赖对象恢复后重放。

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

完整 Venus renderer protocol 仍未完成，但当前实现已通过 Linux 客体 `vulkaninfo --summary`、host-visible Vulkan fill/readback workload、saved-state restore 和 reset workload。下一阶段按以下顺序推进：

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

## 2026-09-28 cleanup 与 buffer 回归边界

- `tools/test-linux-virtio-gpu.ps1` 的清理轮询现在把 direct-session 已失效的 `VBOX_E_INVALID_OBJECT_STATE`/`VBOX_E_VM_ERROR` 视为 VM 已停止，避免 VBoxHeadless 已退出但 `showvminfo` 短暂锁定导致的假失败；真实存活进程和其它注销错误仍会失败。
- 宿主回归 `.build\\windows\\virtio-gpu-validation.json` 使用当前 runtime 通过 56 组，`-IncludeRegistration` 成功；当前 `VBoxDD.dll` SHA256 为 `A5AA4FE295935E384A4569CED70167088878F77715605D10C066D33BAC298095`。
- 客体报告 `.build\\windows\\linux-venus-cleanup-final10\\report.json`：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、6844 次迭代、`cleanupErrors=[]`、`passed=true`，宿主 AMD Radeon 780M / Vulkan 1.4.349，`VBoxSup` 为 `RUNNING`。
- 当前 runtime 的 60 秒压力报告 `.build\\windows\\linux-venus-cleanup-stress60\\report.json`：`guestVulkanWorkloadVerified=true`、42923 次迭代、`cleanupErrors=[]`、`passed=true`；日志无 `stuck`、`expired ring` 或 `ring command rejected`。
- 当前 runtime 的保存恢复报告 `.build\\windows\\linux-venus-cleanup-save5\\report.json`：`saveRestoreVerified=true`、恢复 Vulkan exit 0、初始 workload 3303 次、`cleanupErrors=[]`、`passed=true`。
- 当前 runtime 的重置报告 `.build\\windows\\linux-venus-cleanup-reset5\\report.json`：`resetVerified=true`、reset Vulkan exit 0、初始 workload 3366 次、`cleanupErrors=[]`、`passed=true`。
- 当前 runtime 的双 vCPU 报告 `.build\\windows\\linux-venus-cleanup-cpu2-30\\report.json`：`cpuCount=2`、30 秒 workload 17307 次、`guestVulkanVerified=true`、`cleanupErrors=[]`、`passed=true`；日志无 ring fatal。
- 当前开发包 `.build\\windows\\virtualbox-virtio-gpu-venus-cleanup-final.zip`：257 个文件，ZIP SHA256 `B8123797E6DA7F119CF1083512D6CA0CB270421D73A39F1802E1409A3B030C0A`；包验收的 manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。
- 额外实验表明，单独为 `vkCreateBuffer` 创建未绑定的 host `VkBuffer` 会在约 1024 次 guest queue submit 后触发 fence/ring fatal；上一版 runtime 同一 workload 通过。该路径已撤回，当前 buffer 继续使用已有 blob/resource-backed host `VkBuffer`，不把未绑定的重复对象宣称为完成能力。

## 2026-09-28 buffer memory requirements 语义

- `GET_BUFFER_MEMORY_REQUIREMENTS`（命令 30）和 `GET_BUFFER_MEMORY_REQUIREMENTS2`（命令 145）现在优先查询 blob/resource-backed host `VkBuffer` 的真实 `VkMemoryRequirements`，返回宿主驱动给出的 size、alignment 和 memory type bits；没有已绑定 host buffer 的协议身份仍使用原有有界回退值。
- 宿主回归 `.build\\windows\\virtio-gpu-validation.json` 使用该实现通过 56 组，新增 buffer requirements 查询的非零 size/alignment/type 检查，`-IncludeRegistration` 通过；当前 `VBoxDD.dll` SHA256 为 `F49C3DD98DCD5155AF0A2CA15692225BBE9AF9F509C09B5FBD55E9CAC544511D`。
- 客体报告 `.build\\windows\\linux-venus-buffer-query10\\report.json` 使用同一 runtime 通过 `vulkaninfo` 和 10 秒 host-visible fill/readback workload（7218 次迭代），`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；日志无 `stuck`、`expired ring` 或 `ring command rejected`。
- 当前开发包 `.build\\windows\\virtualbox-virtio-gpu-venus-buffer-query-final.zip` 共 257 个文件，ZIP SHA256 `F328580C8A1235E1120525FA580D2A8D1D4BCEC30739773BDE2015B65E7959A2`；包内 `VBoxDD.dll` SHA256 为 `F49C3DD98DCD5155AF0A2CA15692225BBE9AF9F509C09B5FBD55E9CAC544511D`，manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。

## 2026-09-28 display port 回调

- `PDMIDISPLAYPORT` 的 screenshot、DisplayBlt、CopyRect、动态 video mode 和 dirty-rectangle 通知现在使用当前 scanout/resource shadow；Vulkan image dirty 时先执行 image readback，DisplayBlt 写入后复用现有 host-visible resource sync，不引入独立未绑定的 Vulkan buffer。
- 宿主回归 `.build\\windows\\virtio-gpu-validation.json` 使用最终 origin 修正版 DLL 通过 57 组并包含注册检查；新增 `display port screenshot, blit and copy callbacks` 子测试，`VBoxDD.dll` SHA256 为 `C6F2AE6F5542DB1382F0E5BBC0062D1F9C4AA684CA2DF178809B6E06873B4779`。
- 客体报告 `.build\\windows\\linux-venus-display-port-origin10\\report.json` 使用同一 runtime 通过 `vulkaninfo` 和 10 秒 workload（7164 次迭代），`guestVulkanExit=0`、`guestVulkanWorkloadExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；运行时 DLL SHA256 同为 `C6F2AE6F5542DB1382F0E5BBC0062D1F9C4AA684CA2DF178809B6E06873B4779`，日志无 `stuck`、`expired ring` 或 `ring command rejected`。
- 同一 runtime 的 saved-state 报告 `.build\\windows\\linux-venus-display-port-origin-save5\\report.json`：`saveRestoreVerified=true`、恢复 Vulkan exit 0、初始 workload 3242 次、`guestVulkanVerified=true`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；运行时 DLL SHA256 同为 `C6F2AE6F5542DB1382F0E5BBC0062D1F9C4AA684CA2DF178809B6E06873B4779`。
- 当前开发包 `.build\\windows\\virtualbox-virtio-gpu-venus-display-port-origin-final.zip` 共 257 个文件，ZIP SHA256 `A60A6FE60EE662A60375C3489BB3BC242E2F7EA4465B872C602F552CA4FEED15`；包内 `VBoxDD.dll` SHA256 为 `C6F2AE6F5542DB1382F0E5BBC0062D1F9C4AA684CA2DF178809B6E06873B4779`，manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。

## 2026-09-29 event 生命周期与当前交付包

- `DevVirtioGPU.cpp` 新增 Venus event 命令 42-46：创建时按 `VkEventCreateInfo` 做 64-byte wire 校验，状态操作按 24-byte framing 返回真实 `VkResult`/`VkResult` 状态；host-backed 构建使用宿主 Vulkan `VkEvent`，没有宿主句柄时保留有界软件状态。
- saved-state version 从 20 升为 21；opaque object 表保存 `fEventSet`，恢复时重建 event 并在保存状态为 set 时调用 `vkSetEvent`。
- 宿主 `tools\\test-virtio-gpu.ps1 -TimeoutSeconds 90 -IncludeRegistration` 通过 58 组，报告 `.build\\windows\\virtio-gpu-validation.json`。
- 当前 DLL `.build\\windows\\package-event-final\\bin\\VBoxDD.dll` SHA256 为 `C443444D3AFA0B5FB9D5A9C428DA057AAA375E3709B720A5A527203A6AE43BDB`。
- 当前开发包 `.build\\windows\\virtualbox-virtio-gpu-venus-event-final.zip` 共 257 个文件，ZIP SHA256 `C7CB338F0C3D04008BBC2C7DAA2D8A2FEC2359374A96416B4B2DA82254858C56`；包校验的 manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。
- 客体标准、saved-state 和 reset 报告分别为 `.build\\windows\\linux-venus-event-final-long\\report.json`、`.build\\windows\\linux-venus-event-save5\\report.json`、`.build\\windows\\linux-venus-event-reset5\\report.json`，三者均 `passed=true`、`cleanupErrors=[]`，且 `VBoxSup` 为 `RUNNING`。
- 仍未完成完整 Venus renderer protocol、所有 Vulkan 对象/query、真实桌面显示/分辨率/cursor 端到端验收、8 小时压力、完整性能矩阵和正式 Windows 安装器；本轮证据不应扩大为这些范围已完成。

## 2026-09-29 query pool dispatcher

- `DevVirtioGPU.cpp` 新增 query pool 命令 47-49、171：`vkCreateQueryPool`、`vkDestroyQueryPool`、`vkGetQueryPoolResults`、`vkResetQueryPool`。创建参数按 `VkQueryPoolCreateInfo` 的 pNext、query type/count、pipeline statistics 和 allocator/output marker 校验；宿主 Vulkan 可用时创建真实 `VkQueryPool`。
- `vkGetQueryPoolResults` 读取 `dataSize`、array-size、stride 和 flags，限制结果缓冲在 `VIRTIOGPU_MAX_SUBMIT_BYTES` 内，调用宿主 Vulkan 后返回 `VkResult`、64 位 array-size 和按 4-byte 对齐的结果 blob，避免只推进 reply cursor。
- command stream 查询命令 `vkCmdResetQueryPool`（129）、`vkCmdWriteTimestamp`（130）和 `vkCmdWriteTimestamp2`（205）已校验固定 framing，并使用宿主提交 command buffer 写入真实时间戳；回归随后读取非零 timestamp payload。
- saved-state version 从 21 升为 22；opaque object 类型校验、host handle 析构和恢复 pass 已包含 query pool。宿主 `tstVirtioGPU` 新增 query pool create/reset/get/destroy 与 save/load handle 检查。
- 宿主回归 `.build\windows\virtio-gpu-validation.json` 通过 59 组，`missingGroups=[]`，注册检查通过；当前 `VBoxDD.dll` SHA256 为 `7CEF43BE69538014B4565304F85FBC57E5B47608C742A3FA907D999887227CB7`。
- 当前客体标准报告 `.build\windows\linux-venus-query-final\report.json` 使用同一 runtime 通过 `vulkaninfo` 和 guest workload，`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`；guest 设备为 `Virtio-GPU Venus (AMD Radeon 780M Graphics)`。本轮 query-specific saved-state 脚本重试未生成报告，原因是 VM restore session 在脚本超时后残留，已手动终止临时 VBoxHeadless；宿主 saved-state query handle 检查仍已通过。
- 使用管理员启动 `VBoxSup` 后，`.build\windows\linux-venus-query-save2\report.json` 在当前 runtime 上通过客体 saved-state 验收：`guestVulkanVerified=true`、初始 workload `10361` 次、恢复 workload `10622` 次、`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`cleanupErrors=[]`、`passed=true`；运行时 `VBoxDD.dll` SHA256 为 `7CEF43BE69538014B4565304F85FBC57E5B47608C742A3FA907D999887227CB7`。
- 当前开发包 `.build\windows\virtualbox-virtio-gpu-venus-query-timestamp-final.zip` 共 257 个文件，ZIP SHA256 `85BFD92D5977FE08E6348946381FF2EAEB75DBE279478074F0D26A7FD88D343B`；包内 `VBoxDD.dll` SHA256 为 `7CEF43BE69538014B4565304F85FBC57E5B47608C742A3FA907D999887227CB7`，manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。完整 Venus renderer protocol、真实桌面显示/分辨率/cursor 端到端验收、8 小时压力、完整性能矩阵和正式 Windows 安装器仍未完成。

## 2026-09-29 descriptor set 生命周期

- `DevVirtioGPU.cpp` 新增 Venus descriptor set 命令 77-79：`vkAllocateDescriptorSets` 按单 descriptor pool、单 layout 和单输出 set 做 80-byte framing，宿主 Vulkan 创建真实 `VkDescriptorSet`；`vkFreeDescriptorSets` 释放 opaque set 并校验 pool 归属；`vkUpdateDescriptorSets` 完成动态 write/copy framing，并把常见 sampler/image/buffer descriptor 转换为宿主 `vkUpdateDescriptorSets` 调用。
- saved-state version 从 22 升为 24；descriptor set 保存其 create wire 和宿主 descriptor pool 依赖，descriptor update wire 也保存并在依赖对象恢复后重放。`tstVirtioGPU` 覆盖分配、零更新、直接 save/reset/load 后句柄重建和释放。
- 重新编译 `DevVirtioGPU.cpp`、`tstVirtioGPU.cpp` 并链接后，`.build\windows\virtio-gpu-validation.json` 通过 59 组，`missingGroups=[]`，注册检查通过；当前 `VBoxDD.dll` SHA256 为 `7CEF43BE69538014B4565304F85FBC57E5B47608C742A3FA907D999887227CB7`。
- 当前客体报告 `.build\windows\linux-venus-descriptor-set-save\report.json` 使用同一 runtime、运行中的 `VBoxSup` 通过 guest `vulkaninfo`、15 秒 workload 和 saved-state restore：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、初始 workload `10244` 次、`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 当前开发包 `.build\windows\virtualbox-virtio-gpu-venus-descriptor-set-final.zip` 共 257 个文件，ZIP SHA256 `9BF51963186CE224F67B68C7D5367B46D73BC88D446D1233D952ADB154403244`；包内 `VBoxDD.dll` SHA256 为 `7CEF43BE69538014B4565304F85FBC57E5B47608C742A3FA907D999887227CB7`，manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。完整 descriptor write/copy saved-state 重放、完整 Venus renderer protocol、真实桌面显示/分辨率/cursor 端到端验收、8 小时压力、完整性能矩阵和正式 Windows 安装器仍未完成。

## 2026-09-29 descriptor update framing

- `vkUpdateDescriptorSets` 现在按真实数组大小解析 write/copy payload；常见 sampler、combined/sampled/storage image、input attachment、uniform/storage buffer 及 dynamic buffer descriptor 会转换为宿主结构并调用 `vkUpdateDescriptorSets`。未知 pNext、texel buffer、数组计数不一致、无效 set/resource handle 会拒绝。
- `vkFreeDescriptorSets` 在释放前校验 set 的 descriptor pool 归属；`tstVirtioGPU` 新增截断 update 和错误 pool 负向检查。非空 descriptor update wire 按 slot 保存，并在 saved-state 恢复完成 descriptor pool/layout/set 依赖后重放；释放 descriptor pool/layout/set 时清理缓存，避免恢复旧句柄。
- 当前 VBoxDD 重新链接后宿主回归 `.build\windows\virtio-gpu-validation.json` 通过 59 组，`missingGroups=[]`；当前客体报告 `.build\windows\linux-venus-descriptor-update-save\report.json` 使用同一 runtime 通过 `vulkaninfo`、15 秒 workload（10211 次）和 saved-state restore：`guestVulkanVerified=true`、`guestVulkanWorkloadVerified=true`、`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`。
- 当前开发包 `.build\windows\virtualbox-virtio-gpu-venus-descriptor-update-final.zip` 共 257 个文件，ZIP SHA256 `B713CE4954C7BC91967593F87B77CCC90ED6DB204217BD1BDCAB9465E694EBA4`；包内 `VBoxDD.dll` SHA256 为 `7CEF43BE69538014B4565304F85FBC57E5B47608C742A3FA907D999887227CB7`，包校验通过。完整 descriptor update saved-state 重放、完整 Venus renderer protocol、真实桌面显示/分辨率/cursor 端到端验收、8 小时压力、完整性能矩阵和正式 Windows 安装器仍未完成。

## v24 write/copy 当前修正

- 上方 descriptor set/update 的 v23 记录是历史阶段；当前 saved-state version 为 24，非空 descriptor update wire 已保存，并在 descriptor pool/layout/set 依赖恢复后重放。
- 当前客体报告为 `.build\windows\linux-venus-descriptor-update-save-v24\report.json`：guest Vulkan、15 秒 workload（10646 次）和 saved-state restore 均通过，`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`。
- 当前开发包为 `.build\windows\virtualbox-virtio-gpu-venus-descriptor-update-v24-write-copy-final.zip`，共 257 个文件，ZIP SHA256 `FD6DA60092A2416DDF3885AD2D8AEBD7950BC3987105A1373BF74CD91CB629E8`；包内 `VBoxDD.dll` SHA256 为 `7CEF43BE69538014B4565304F85FBC57E5B47608C742A3FA907D999887227CB7`，manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。
- 宿主 `tstVirtioGPU` 已构造真实 sampler descriptor layout/pool/两个 set，验证非空 `vkUpdateDescriptorSets` 的 write 与 copy payload，并验证 save/reset/load 后两个宿主句柄恢复及 update 重放；`.build\windows\virtio-gpu-validation.json` 为 59 组通过、`missingGroups=[]`、注册检查通过。当前 guest workload 仍未单独覆盖非空 descriptor payload。完整 Venus renderer protocol、真实桌面显示/分辨率/cursor 端到端验收、8 小时压力、完整性能矩阵和正式 Windows 安装器仍未完成。

## 2026-09-29 EDID、默认显示模式与 scanout saved-state

- 修正 `GET_EDID` 的标准 base-block header 和 1024x768@60 DTD；scanout 0 在设备构造和资源清理后默认保留 1024x768，`GET_DISPLAY_INFO` 不再被通用响应清零，并在有默认尺寸时报告 enabled。
- 修正 saved-state resource 校验：普通 2D resource 可以在 `cBacking=0` 时保存宿主像素存储；只有存在 backing 条目时才要求 backing 容量覆盖 resource 数据。这样客体的无 backing scanout resource 可以正常恢复。
- 最终宿主回归 `.build\windows\virtio-gpu-validation.json` 通过 59 组，`missingGroups=[]`，`-IncludeRegistration` 通过；最终 `VBoxDD.dll` SHA256 为 `D32B2CBCB4EEB9AB79C7DD1E12686E8B2949BBA869F885A6E19CBDF3571EEC1B`。
- 使用最终开发包的客体报告 `.build\windows\linux-venus-display-edid-ssm-save-final\report.json`：`guestReady=true`、`sshReady=true`、`drmDriverBound=true`、`displayModeVerified=true`，客体实际报告 `1024x768`/`640x480`，`guestVulkanVerified=true`、15 秒 workload `10217` 次、`saveRestoreVerified=true`、`saveRestoreVulkanExit=0`、`vboxSupState=RUNNING`、`cleanupErrors=[]`、`passed=true`。
- 同一最终开发包的 reset 报告 `.build\windows\linux-venus-display-edid-ssm-reset-final\report.json`：`displayModeVerified=true`、`guestVulkanVerified=true`、10 秒 workload `7147` 次、`resetVerified=true`、`resetVulkanExit=0`、`cleanupErrors=[]`、`passed=true`。
- 当前开发包 `.build\windows\virtualbox-virtio-gpu-venus-display-edid-ssm-final.zip` 共 257 个文件，ZIP SHA256 `E8B18D8502205A18D52772BB86196F1CDF4E023C32E1C295EAA4C320087FF3CE`；包内 `VBoxDD.dll` SHA256 为 `D32B2CBCB4EEB9AB79C7DD1E12686E8B2949BBA869F885A6E19CBDF3571EEC1B`，manifest、版本、VirtIO-GPU/Venus 回归和 PE/VMMR0 加载均通过。
- 客体串口仍记录 `response 0x1205` 对命令 `0x105`/`0x106`（resource backing/2D transfer）错误；当前证据证明 DRM 模式枚举、Vulkan、workload 和 saved-state restore，不证明完整桌面 framebuffer 内容、cursor 或任意分辨率切换已经端到端完成。完整 Venus renderer protocol、8 小时压力、完整性能矩阵和正式 Windows 安装器仍未完成。

## 2026-09-29 guest framebuffer backing segments

- Linux 客体的 1024x768 scanout 会提交 137 个 backing segment；原来的 64 段宿主上限会拒绝 `RESOURCE_ATTACH_BACKING`，随后 `TRANSFER_TO_HOST_2D` 返回 `0x1205`。
- `VIRTIOGPU_MAX_BACKING_ENTRIES` 提升为 256，并保留溢出、总容量和 saved-state 边界校验；宿主回归新增 65 段 attach/transfer 用例。
- saved-state version 从 24 升为 25，因为资源 backing 表的序列化大小随上限扩大。
- 当前宿主回归 `.build\\windows\\virtio-gpu-validation.json` 通过 60 组，`missingGroups=[]`，注册检查通过；当前 `VBoxDD.dll` SHA256 为 `28B757E1BC010308CC1FF035AE0D3B0C151C78B13ADA5665035449DD0204C666`。
- `.build\\windows\\linux-venus-transfer-fixed3\\report.json` 使用同一 runtime 通过客体 DRM/VirtIO-GPU 启动与 1024x768 模式枚举；该报告未运行 Vulkan。
- `.build\\windows\\linux-venus-transfer-fixed5c\\report.json` 使用同一 runtime 通过 `vulkaninfo`、5 秒 workload（3539 次）和清理；日志不再出现 `0x105`/`0x106`，但仍记录独立的 legacy `SUBMIT_3D` `0x1205`（命令 `0x207`），该路径仍需后续协议补齐。
- 一次 workload 重试曾因 Venus ring 的 `QUEUE_SUBMIT` 解析在第 1024 次附近触发 fatal；同一修订后的复验通过，不能把单次失败扩大为稳定性结论。

只提交本阶段相关文件，保留测试报告在 `.build` 下，不要提交临时 VM 密钥、磁盘或 core dump。
