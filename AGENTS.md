# AGENTS.md — virtualbox-virtio-gpu-venus handoff

## 项目目标

在 Windows 宿主上编译 VirtualBox，并通过 VirtIO-GPU/Venus 将宿主机 Vulkan GPU 能力提供给 Linux 客体。方案不使用 PCIe 直通；依赖 VirtIO-GPU、共享显存 BAR 和 Vulkan 命令转发。

## 当前仓库状态

- 工作目录：`D:\code\virtualbox-virtio-gpu-venus`
- 分支：`main`
- 远端：`git@github.com:wso4133560/virtualbox-virtio-gpu-venus.git`
- 最新已推送提交：`cd0ccca1b849c11057bf52a2f69c67dd72ca5c2f`
- 上一个相关提交：`cc61cf9c`（Venus context、blob map 修正）
- 当前工作树在交接时应保持干净；开始新工作先执行 `git status --short`。

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

说明：`tools/build-windows.ps1` 是完整构建入口，但当前 checkout 缺少部分可选源码目录（例如 libssh/keygen），完整目标可能被这些依赖阻塞。上面的 `kmk` 增量目标可以成功编译 `DevVirtioGPU.cpp`、链接 `VBoxDD.dll` 并生成 `tstVirtioGPU.exe`。Qt6 缺失提示目前是 warning。

## 已验证命令

Windows 宿主回归：

```powershell
.\tools\test-virtio-gpu.ps1 -TimeoutSeconds 60
```

当前回归大部分项目通过；blob resource/map 子测试仍报告 3 个失败（测试行 1773、1777、1815），不要据此宣称全量回归通过。

Windows 宿主 Vulkan/GPU 利用率：

```powershell
.\tools\test-windows-gpu-utilization.ps1
```

已验证宿主设备为 AMD Radeon 780M Graphics，Vulkan 1.4.349；历史报告：`.build\windows\gpu-utilization-validation.json`。该结果只证明宿主 Vulkan 路径和 GPU 利用率，不证明 Linux 客体 Vulkan 已成功。

Linux 客体测试：

```powershell
.\tools\test-linux-virtio-gpu.ps1 `
  -ImagePath '.build\images\noble-20260911-amd64.img' `
  -GpuBackend venus `
  -VerifyGuestVulkan `
  -ReportDirectory '.build\windows\linux-venus-next'
```

镜像必须匹配 `tools\linux-test-image.json` 的 pinned bytes/SHA256。脚本会创建临时 VM、SSH 转发和报告目录，并在结束时清理 VM。

## 当前已完成

1. Windows VirtIO-GPU 设备、PCI shared-memory capability、host-visible BAR。
2. 宿主 Vulkan loader/device/queue 初始化和资源同步。
3. HOST3D blob 创建、映射/取消映射、Vulkan buffer/image backing。
4. 多个 Vulkan transfer/clear/barrier/blit/fill/update 命令及批处理。
5. `CTX_CREATE` 现在兼容 Venus capset id `4`，同时保留旧测试使用的 init 值 `1`。
6. `RESOURCE_MAP_BLOB`/`UNMAP_BLOB` 已按“common header 已消费后只读 8-byte body”修正。
7. 增加了 Venus ring 元数据、ring/virtqueue seqno 控制命令识别，以及 BO-only submit 的共享 ring head/status 推进。

## 当前未完成和已知限制

Linux 客体当前报告：

- `guestReady=true`
- `sshReady=true`
- `drmDriverBound=true`
- `hostVisible=true`
- `guestVulkanVerified=false`
- `vulkaninfo` 退出码 `134`
- 日志：`MESA-VIRTIO: stuck in ring seqno wait ...`

最近报告示例：`.build\windows\linux-venus-final-ring\report.json` 和 `guest-vulkan.log`。

主要未完成项是完整 Venus renderer protocol：

1. 正确处理 `vkCreateRingMESA` 的所有字段并持久化 ring 状态。
2. 实现 `vkSetReplyCommandStreamMESA`，保存 reply resource/offset/size。
3. 实现 `vkExecuteCommandStreamsMESA`：从共享 blob 读取 command stream，解析/执行命令，并写入 reply stream。
4. 实现 reply stream 的命令类型、返回值和错误码，不能只返回 `VIRTIOGPU_RESP_OK_NODATA`。
5. 正确处理 `vkNotifyRingMESA`、`vkWriteRingExtraMESA`、`vkSubmitVirtqueueSeqnoMESA`、`vkWait*SeqnoMESA` 的 ring/head/tail/status 语义。
6. 将 ring 状态加入 saved-state save/load，避免快照恢复丢失 ring 元数据。
7. 修复或重新厘清 blob map 回归测试的 3 个失败。

当前的“推进 head/tail/status”是防止死等的 bring-up 逻辑，不代表命令已经在宿主 GPU 上执行，也不应作为 Vulkan 正常工作的最终实现。

## 调试方法

- 客体 Vulkan 调试脚本设置 `VN_DEBUG=init` 和 `VK_LOADER_DEBUG=all`，完整日志在报告目录 `guest-vulkan.log`。
- 宿主 VM 日志在报告目录 `VBox.log`；串口日志为 `serial.log`。
- 重点搜索：

```powershell
rg -n 'stuck|failed|MESA-VIRTIO|SUBMIT_3D|virtio-gpu:' .build\windows\linux-venus-*\guest-vulkan.log .build\windows\linux-venus-*\VBox.log
```

- Mesa 协议定义位置：
  `VirtualBox-7.2.6\src\VBox\Additions\3D\mesa\mesa-24.0.2\src\virtio\venus-protocol\vn_protocol_driver_transport.h`
- ring 实现位置：
  `...\src\virtio\vulkan\vn_ring.c`
- 宿主 VirtIO-GPU 实现：
  `VirtualBox-7.2.6\src\VBox\Devices\VirtIO\DevVirtioGPU.cpp`
- ring 布局来自 Mesa `vn_ring_get_layout()`：head offset 0、tail offset 64、status offset 128，均按 64-byte 对齐；这只是当前 Mesa 版本的布局事实，最终应使用 `vkCreateRingMESA` 传入的字段。

## Windows COM/驱动问题

`tools\register-windows-runtime.ps1` 成功只表示 runtime COM 注册验证通过。若 `VBoxManage.exe list hostinfo` 报 `ERROR_FILE_NOT_FOUND`，先确认使用同一 runtime 目录下的 `VBoxManage.exe`、`VBoxSVC.exe`、`VBoxC.dll` 和 `VBoxDD.dll`，不要混用系统安装版 VirtualBox。

`sc.exe query VBoxSup` 返回 1060 表示 VBoxSup 服务未安装。驱动测试签名、VBoxSup/VBoxSup-inf 安装和管理员权限属于独立阶段；不要把 COM 注册成功当作驱动已安装。

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
