# virtualbox-virtio-gpu-venus
实现基于virtualbox-7.2.6实现virtio-gpu + venus的GPU透传方案


开发目标是通过 VirtIO-GPU/Venus 转发 Vulkan 工作，复用宿主机 GPU，不采用 PCIe 独占直通。

- [开发计划](doc/开发计划.md)：完整阶段与验收要求。
- [实现进度与复现命令](doc/实现进度.md)：当前已验证范围、未完成项和产物记录。

当前完成 Windows 核心开发构建及 VirtIO-GPU 控制队列子阶段；尚未提供客户机图形显示或 Venus 加速。
