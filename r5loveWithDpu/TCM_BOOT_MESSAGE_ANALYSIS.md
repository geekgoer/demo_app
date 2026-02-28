# 🔍 "RPU boot from TCM" 消息的真相

## 问题分析

### 现象
- ✅ Device tree 中没有 `srams` 属性
- ✅ 编译的 dtb 正确
- ❌ dmesg 仍显示 "RPU boot from TCM"
- ❌ XSCT 显示 PC = 0xe774（TCM 地址）

### 关键发现

**"RPU boot from TCM" 这条消息可能是误导性的！**

让我们看看 Linux 内核驱动 `zynqmp_r5_remoteproc.c` 的逻辑：

```c
// 驱动判断启动模式的逻辑
if (rpu->tcm_bank_count > 0) {
    dev_info(dev, "RPU boot from TCM.\n");
} else {
    dev_info(dev, "RPU boot from DDR.\n");
}
```

**问题**：即使没有 `srams` 属性，如果驱动检测到 TCM 存在，也可能显示 "boot from TCM"！

### 真正的问题：ELF 加载地址

让我们检查 ELF 文件的加载地址：

```bash
readelf -l r5app.elf
```

**关键信息**：
- Entry point: 0x3ed00000 ✅
- LOAD segment: VirtAddr=0x3ed00000, PhysAddr=0x3ed00000 ✅

**但是**：Linux remoteproc 可能有自己的加载逻辑！

## 🔍 深度诊断

### 检查 1: 查看 remoteproc 的内存映射

在板子上执行：

```bash
# 查看 remoteproc 的内存区域
cat /sys/kernel/debug/remoteproc/remoteproc0/resource_table

# 或者查看映射
cat /sys/kernel/debug/remoteproc/remoteproc0/carveout_memories
```

### 检查 2: 对比 r5_amp_echo.elf

**关键问题**：为什么 r5_amp_echo 能工作，但 r5app 不行？

让我们对比两个 ELF：

```bash
# 对比入口点
readelf -h r5_amp_echo.elf | grep Entry
readelf -h r5app.elf | grep Entry

# 对比程序头
readelf -l r5_amp_echo.elf
readelf -l r5app.elf

# 对比节头
readelf -S r5_amp_echo.elf | grep -E "\.text|\.vectors|\.data"
readelf -S r5app.elf | grep -E "\.text|\.vectors|\.data"
```

### 检查 3: 查看 resource_table

```bash
# 检查 resource_table 的位置
readelf -s r5app.elf | grep resource_table

# 查看 resource_table 的内容
readelf -x .resource_table r5app.elf
```

## 🎯 可能的真正原因

### 原因 A: 启动代码问题

**XSCT 显示 PC = 0xe774**，这不是 0x3ed00000！

可能的情况：
1. R5 确实从 DDR 启动（0x3ed00000）
2. 但代码执行后跳转到了 TCM（0xe774）
3. 可能是启动代码或 ThreadX 初始化有问题

### 原因 B: VBAR 设置时机问题

我们在 `main()` 中设置 VBAR，但可能：
1. 在 `main()` 之前就触发了中断
2. 或者 `init_platform()` 修改了 VBAR
3. 或者 ThreadX 初始化时重置了 VBAR

### 原因 C: 启动代码在 TCM

可能 linker script 中的 `.boot` 段被放在了 TCM！

检查 linker script：

```ld
.vectors : {
   KEEP (*(.vectors))
   *(.boot)  ← 这个可能有问题！
} > psu_r5_ddr_amp
```

## 🔧 解决方案

### 方案 1: 在启动代码中设置 VBAR

不要在 `main()` 中设置，而是在 `_boot` 或 `_vector_table` 中设置。

**修改启动汇编代码**（如果有的话）：

```assembly
_boot:
    # 设置 VBAR 为向量表地址
    ldr r0, =_vector_table
    mcr p15, 0, r0, c12, c0, 0

    # 继续启动
    ...
```

### 方案 2: 检查 platform_init

`init_platform()` 可能修改了内存配置。

尝试在 `main()` 中调整顺序：

```c
int main()
{
    /* 1. 先设置 VBAR */
    extern u32 _vector_table;
    mtcp("p15, 0, %0, c12, c0, 0", (u32)&_vector_table);

    /* 2. 禁用所有中断 */
    __asm__ __volatile__("cpsid if");

    /* 3. 初始化平台 */
    init_platform();

    /* 4. 再次确认 VBAR */
    mtcp("p15, 0, %0, c12, c0, 0", (u32)&_vector_table);

    /* 5. 设置定时器（会启用中断）*/
    setup_timer_interrupt();

    /* 6. 进入 ThreadX */
    tx_kernel_enter();
}
```

### 方案 3: 使用 r5_amp_echo 的 linker script

直接使用 r5_amp_echo 的 linker script，因为它能工作！

```bash
cp r5_amp_echo/src/lscript.ld r5loveWithDpu/r5app/src/lscript.ld.amp
```

## 📋 立即执行的诊断

### 1. 对比两个 ELF 文件

```bash
# 在 Windows 上
readelf -l D:/files/VitisProject/demoAmp/r5_amp_echo/Debug/r5_amp_echo.elf > echo.txt
readelf -l D:/files/VitisProject/demoAmp/r5loveWithDpu/r5app/Debug/r5app.elf > app.txt

# 对比
diff echo.txt app.txt
```

### 2. 检查启动流程

在 `main()` 第一行添加调试输出：

```c
int main()
{
    xil_printf("=== MAIN START ===\r\n");
    xil_printf("PC should be around 0x3ed00000\r\n");

    extern u32 _vector_table;
    xil_printf("_vector_table = 0x%08x\r\n", (u32)&_vector_table);

    // 读取当前 VBAR
    u32 current_vbar;
    __asm__ __volatile__("mrc p15, 0, %0, c12, c0, 0" : "=r" (current_vbar));
    xil_printf("Current VBAR = 0x%08x\r\n", current_vbar);

    // 设置 VBAR
    mtcp("p15, 0, %0, c12, c0, 0", (u32)&_vector_table);

    // 再次读取确认
    __asm__ __volatile__("mrc p15, 0, %0, c12, c0, 0" : "=r" (current_vbar));
    xil_printf("New VBAR = 0x%08x\r\n", current_vbar);

    // ... 其余代码
}
```

### 3. 连接 R5 的 UART1

**这是最重要的**！你需要看到 R5 的串口输出才能知道发生了什么！

---

**请先执行这些诊断，特别是连接 UART1 查看 R5 的输出！**
