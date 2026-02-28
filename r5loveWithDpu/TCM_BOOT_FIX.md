# 🎯 R5 从 TCM 启动问题 - 已解决

## 问题诊断

### 症状
- ✅ R5 成功启动（dmesg 显示 "remote processor r5@0 is now up"）
- ✅ ELF 文件正确（Entry point 0x3ed00000）
- ❌ XSCT 显示 PC = 0x0000e774（在 TCM 中，不在 DDR）
- ❌ `_tx_timer_system_clock` 始终为 0
- ❌ ThreadX 没有运行

### 根本原因

**dmesg 输出的关键信息**：
```
r5@0: RPU boot from TCM.
```

Linux remoteproc 驱动配置 R5 从 **TCM 启动**，而不是从 **DDR 启动**！

**为什么会这样？**

在 `system-user.dtsi` 第 53 行：
```dts
srams = <&tcm_0_a &tcm_0_b>;
```

这个属性告诉 Linux：
1. R5 使用 TCM 作为主要内存
2. 启动时，将 ELF 的代码段加载到 TCM
3. 设置 R5 的 PC 指向 TCM 地址

**但是**：
- 我们的 linker script 把所有代码放在 DDR (0x3ed00000)
- TCM 只有 64KB (ATCM) + 64KB (BTCM) = 128KB
- 我们的 r5app.elf 有 1.2MB，根本放不进 TCM！

**结果**：
- Linux 尝试把代码加载到 TCM，但空间不够
- 或者只加载了部分代码
- R5 从 TCM 启动，执行到无效代码
- 系统在 TCM 中"迷路"，无法跳转到 DDR

## 解决方案

### 修改 device tree

**文件**：`petalinux-files/system-user.dtsi`

**修改**：注释掉 `srams` 行（第 53 行）

```dts
r5_0: r5@0 {
    #address-cells = <1>;
    #size-cells = <1>;
    ranges;
    memory-region = <&rproc_0_reserved>,
                   <&rpu0vdev0vring0>,
                   <&rpu0vdev0vring1>,
                   <&rpu0vdev0buffer>;
    pnode-id = <0x7>;
    mboxes = <&ipi_mailbox_rpu0 0>, <&ipi_mailbox_rpu0 1>;
    mbox-names = "tx", "rx";
    /* 注释掉 srams，让 R5 从 DDR 启动而不是 TCM */
    /* srams = <&tcm_0_a &tcm_0_b>; */
};
```

### 重新编译 PetaLinux

**在 Ubuntu 虚拟机中**：

```bash
cd /path/to/petalinux-project

# 1. 复制修改后的 device tree
cp /path/to/system-user.dtsi project-spec/meta-user/recipes-bsp/device-tree/files/

# 2. 清理并重新编译 device tree
petalinux-build -c device-tree -x cleansstate
petalinux-build -c device-tree

# 3. 重新打包 BOOT.BIN 和 image.ub
petalinux-build

# 4. 生成启动文件
petalinux-package --boot --fsbl images/linux/zynqmp_fsbl.elf \
                         --u-boot images/linux/u-boot.elf \
                         --pmufw images/linux/pmufw.elf \
                         --fpga images/linux/system.bit \
                         --force
```

### 更新 SD 卡

```bash
# 复制新的文件到 SD 卡
cp images/linux/BOOT.BIN /media/sd_card/
cp images/linux/image.ub /media/sd_card/
cp images/linux/boot.scr /media/sd_card/

# 同步
sync
```

### 重启板子并测试

```bash
# 板子重启后，在 Linux 中
echo r5app.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state

# 检查启动信息
dmesg | tail -20
```

**预期输出**：
```
remoteproc remoteproc0: Booting fw image r5app.elf, size 1194936
r5@0: RPU boot from DDR.  ← 注意：应该是 DDR，不是 TCM！
virtio_rpmsg_bus virtio0: rpmsg host is online
virtio_rpmsg_bus virtio0: creating channel rpmsg-openamp-demo-channel addr 0x0
remoteproc remoteproc0: remote processor r5@0 is now up
```

### 使用 XSCT 验证

```tcl
connect
ta 7
stop
rrd pc  # 应该在 0x3ed00000 - 0x3ee00000 范围内

# 检查 ThreadX 心跳
mrd 0x3ed218d8
# 等待 5 秒
mrd 0x3ed218d8  # 值应该增加了约 500

con
```

## 技术细节

### TCM vs DDR 启动模式

**TCM 启动**：
- 优点：速度快（零等待周期）
- 缺点：空间小（只有 128KB）
- 适用：小型裸机程序

**DDR 启动**：
- 优点：空间大（可以有几 MB）
- 缺点：速度稍慢（有内存访问延迟）
- 适用：大型程序、RTOS 应用

### Linux remoteproc 的启动逻辑

1. **读取 device tree**：
   - 如果有 `srams` 属性 → TCM 启动
   - 如果没有 `srams` → DDR 启动（使用 memory-region）

2. **加载 ELF**：
   - 解析 ELF 的 LOAD 段
   - 根据 PhysAddr 加载到对应内存

3. **启动 R5**：
   - 设置 R5 的 PC 为 ELF 的 Entry point
   - 释放 R5 复位

### 为什么之前 r5_amp_echo 能工作？

可能的原因：
1. r5_amp_echo.elf 比较小（785KB），可能 Linux 能够处理
2. 或者之前的 device tree 没有 `srams` 属性
3. 或者 r5_amp_echo 的 linker script 同时支持 TCM 和 DDR

## 学习要点

### 1. Device Tree 的作用
- 描述硬件配置
- 告诉 Linux 如何管理硬件
- 影响驱动的行为

### 2. ELF 加载过程
- Linux 读取 ELF 的 Program Headers
- 根据 PhysAddr 加载到物理内存
- 设置 CPU 的 PC 寄存器

### 3. 内存架构
- TCM：Tightly Coupled Memory（紧耦合内存）
- DDR：外部 DRAM
- 不同的性能和容量特性

### 4. 调试技巧
- 使用 dmesg 查看内核日志
- 使用 XSCT 检查 CPU 状态
- 对比工作和不工作的配置

## 参考资料

1. **Linux remoteproc 文档**
   - `Documentation/devicetree/bindings/remoteproc/xlnx,zynqmp-r5-remoteproc.yaml`

2. **Xilinx Wiki**
   - https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18842504/RPU

3. **Device Tree Specification**
   - https://www.devicetree.org/

---

**状态**：✅ 已修复 - 需要重新编译 PetaLinux 并更新 SD 卡
**日期**：2026-02-05
