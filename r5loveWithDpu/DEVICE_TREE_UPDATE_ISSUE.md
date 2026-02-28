# 🔍 Device Tree 没有更新问题排查

## 问题
修改了 `system-user.dtsi.v2` 并重新编译，但 dmesg 仍然显示：
```
r5@0: RPU boot from TCM.
```

## 可能的原因

### 原因 1: PetaLinux 没有使用修改后的文件

**检查步骤**：

在 Ubuntu 虚拟机中：

```bash
cd /path/to/petalinux-project

# 1. 检查 recipes 中的文件是否更新
cat project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi | grep -A 2 "srams"

# 应该看到注释掉的 srams 行
```

**如果没有注释**：
```bash
# 重新复制文件
cp /mnt/shared/petalinux-files/system-user.dtsi.v2 \
   project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi

# 确认复制成功
cat project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi | grep -A 2 "srams"
```

### 原因 2: Device Tree 编译缓存问题

**解决方案**：

```bash
# 强制清理 device tree
petalinux-build -c device-tree -x mrproper

# 重新编译
petalinux-build -c device-tree

# 检查生成的 dtb 文件
ls -lh images/linux/system.dtb
```

### 原因 3: 编译后的 dtb 文件没有正确复制到 SD 卡

**检查步骤**：

```bash
# 在 Ubuntu 中检查编译后的 dtb
dtc -I dtb -O dts images/linux/system.dtb | grep -A 5 "r5@0"

# 应该看不到 srams 行
```

**如果看到 srams**：说明编译没有生效，需要重新编译。

**如果没有 srams**：说明编译正确，但 SD 卡没有更新。

### 原因 4: SD 卡上的文件没有更新

**可能情况**：
- 复制了错误的文件
- SD 卡挂载点不对
- 文件系统缓存问题

**解决方案**：

```bash
# 1. 确认 SD 卡挂载点
mount | grep sd

# 2. 强制同步并卸载
sync
umount /media/sd_card

# 3. 重新挂载
mount /dev/sdX1 /media/sd_card  # 替换 sdX1 为实际设备

# 4. 复制文件
cp images/linux/system.dtb /media/sd_card/
ls -lh /media/sd_card/system.dtb

# 5. 强制同步
sync
sync
sync

# 6. 卸载
umount /media/sd_card
```

### 原因 5: 板子使用的是 image.ub，不是 system.dtb

**关键问题**：PetaLinux 可能把 device tree 打包进了 `image.ub`！

**检查方法**：

在板子的 Linux 中：
```bash
# 查看当前使用的 device tree
cat /proc/device-tree/zynqmp_r5_rproc/r5@0/srams
# 如果这个文件存在，说明 srams 还在！
```

**解决方案**：

```bash
# 在 Ubuntu 中重新打包 image.ub
cd /path/to/petalinux-project

# 1. 清理
petalinux-build -c kernel -x mrproper
petalinux-build -c device-tree -x mrproper

# 2. 重新编译
petalinux-build

# 3. 复制 image.ub（不是 system.dtb）
cp images/linux/image.ub /media/sd_card/
sync
```

## 🎯 推荐的完整重新编译流程

```bash
cd /path/to/petalinux-project

# 1. 确认源文件正确
cat project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi | grep -A 2 "srams"
# 应该看到注释

# 2. 如果不对，重新复制
cp /mnt/shared/petalinux-files/system-user.dtsi.v2 \
   project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi

# 3. 完全清理
petalinux-build -c device-tree -x mrproper
petalinux-build -c kernel -x mrproper

# 4. 重新编译整个项目
petalinux-build

# 5. 重新打包 BOOT.BIN
petalinux-package --boot --fsbl images/linux/zynqmp_fsbl.elf \
                         --u-boot images/linux/u-boot.elf \
                         --pmufw images/linux/pmufw.elf \
                         --fpga images/linux/system.bit \
                         --force

# 6. 检查生成的文件
ls -lh images/linux/BOOT.BIN
ls -lh images/linux/image.ub
ls -lh images/linux/system.dtb

# 7. 验证 dtb 内容
dtc -I dtb -O dts images/linux/system.dtb | grep -A 10 "r5@0"
# 应该看不到 srams 行

# 8. 复制到 SD 卡
umount /media/sd_card
mount /dev/sdX1 /media/sd_card

cp images/linux/BOOT.BIN /media/sd_card/
cp images/linux/image.ub /media/sd_card/
cp images/linux/boot.scr /media/sd_card/

# 9. 强制同步
sync
sync
sync

# 10. 卸载
umount /media/sd_card
```

## 🔍 在板子上验证

重启板子后，在 Linux 中执行：

```bash
# 1. 检查 device tree 中是否还有 srams
ls -la /proc/device-tree/zynqmp_r5_rproc/r5@0/

# 如果看到 "srams" 文件，说明 device tree 没有更新
# 如果没有 "srams" 文件，说明更新成功

# 2. 查看完整的 device tree
cat /proc/device-tree/zynqmp_r5_rproc/r5@0/compatible
# 应该显示 "xlnx,zynqmp-r5-remoteproc-1.0"

# 3. 启动 R5 并查看日志
echo r5app.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state
dmesg | tail -5

# 应该看到：
# r5@0: RPU boot from DDR.  ← 不是 TCM！
```

## 📋 快速诊断命令

**在板子上执行**：
```bash
# 检查是否有 srams 属性
ls /proc/device-tree/zynqmp_r5_rproc/r5@0/srams 2>/dev/null && echo "srams 还在！" || echo "srams 已删除"
```

如果输出 "srams 还在！"，说明 device tree 没有更新。

---

**请按照上面的完整流程重新编译，特别注意：**
1. 使用 `mrproper` 完全清理
2. 复制 `image.ub`，不只是 `system.dtb`
3. 在板子上用 `/proc/device-tree` 验证
