# 🔧 VBAR 寄存器修复 - ThreadX 中断问题解决

## 问题现象

通过 XSCT 调试发现：
- `_tx_timer_system_clock` 始终为 0（ThreadX 定时器没有工作）
- PC 停在 0x0000962c（TCM 地址），而不是 DDR 的 0x3ed00000
- CPSR = 0x000001db，末尾 `1b` 表示 **Undefined Mode**（未定义指令异常）
- 系统已经崩溃，无法响应中断

## 根本原因

这是 ARM Cortex-R5 在 AMP 配置下的经典问题：

1. **向量表位置不匹配**：
   - 代码和向量表都在 DDR (0x3ed00000)
   - 但 VBAR 寄存器默认值是 0x00000000（指向 TCM）

2. **中断触发时的崩溃链**：
   ```
   定时器中断触发
   → CPU 根据 VBAR=0x0 去查找中断向量
   → 在 0x0 处读到无效指令
   → 触发 Undefined Instruction 异常
   → 进入 Undefined Mode (CPSR=0x1b)
   → 系统死锁，无法恢复
   ```

3. **为什么 `_tx_timer_system_clock` 为 0**：
   - ThreadX 的心跳依赖定时器中断
   - 中断无法正确执行，所以计数器永远不会增加

## 解决方案

在 `main()` 函数的**第一行**设置 VBAR 寄存器指向 DDR 中的向量表。

### 代码修改

**文件**: `r5loveWithDpu/r5app/src/threadx_amp_demo.c`

#### 1. 添加头文件（第 17 行）：
```c
#include "xpseudo_asm.h"  /* For VBAR register access */
```

#### 2. 修改 main 函数（第 362-382 行）：
```c
int main()
{
    /* CRITICAL: Set VBAR to point to vector table in DDR */
    /* Without this, interrupts will crash the system */
    extern u32 _vector_table;
    mtcp(XREG_CP15_VBAR, (u32)&_vector_table);

    xil_printf("\r\n");
    xil_printf("===========================================\r\n");
    xil_printf("Starting ThreadX + AMP Demo\r\n");
    xil_printf("VBAR set to: 0x%08x\r\n", (u32)&_vector_table);
    xil_printf("===========================================\r\n");

    init_platform();

    /* Setup timer interrupt before entering ThreadX */
    setup_timer_interrupt();

    xil_printf("Timer interrupt configured\r\n");
    xil_printf("Entering ThreadX kernel...\r\n\r\n");

    /* Enter ThreadX kernel */
    tx_kernel_enter();

    /* Should never reach here */
    return 0;
}
```

## 技术细节

### VBAR 寄存器
- **全称**: Vector Base Address Register（向量基地址寄存器）
- **作用**: 告诉 CPU 中断向量表的起始地址
- **默认值**: 0x00000000（复位后）
- **必须设置**: 当向量表不在地址 0 时

### mtcp 函数
```c
mtcp(XREG_CP15_VBAR, address);
```
- `mtcp`: Move To CoProcessor（写协处理器寄存器）
- `XREG_CP15_VBAR`: CP15 协处理器的 VBAR 寄存器
- 这是访问 ARM 系统寄存器的标准方法

### _vector_table 符号
- 由链接器脚本定义（lscript.ld.amp）
- 指向 `.vectors` section 的起始地址
- 在我们的配置中，应该是 0x3ed00000

## 验证步骤

### 1. 重新编译
```
右键 r5app → Clean Project
右键 r5app → Build Project
```

### 2. 部署到板子
```bash
# 复制到 SD 卡
cp r5app.elf /media/sd_card/lib/firmware/

# 在板子上加载
echo r5app.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state
```

### 3. 使用 XSCT 验证

连接 XSCT 并检查：

```tcl
# 连接到 R5
connect
targets
ta 7  # Cortex-R5 #0

# 读取 VBAR 寄存器（通过 CP15）
# 注意：XSCT 可能不支持直接读 VBAR，但可以通过内存验证

# 检查 _tx_timer_system_clock 是否在增加
mrd 0x3ed218d8
# 等待几秒
mrd 0x3ed218d8
# 应该看到值在增加！

# 检查 PC 是否在正常范围
stop
rrd pc
# 应该在 0x3ed00000 - 0x3ee00000 范围内

# 检查 CPSR
rrd cpsr
# 应该是正常模式（如 0x13 = SVC mode），不是 0x1b
```

### 4. 预期输出

在串口上应该看到：
```
===========================================
Starting ThreadX + AMP Demo
VBAR set to: 0x3ed00000
===========================================
Timer interrupt configured
Entering ThreadX kernel...

ThreadX application defined successfully
Demo thread started
RPMsg thread started

===========================================
ThreadX + AMP Demo on R5
===========================================
Creating rpmsg endpoint...
RPMsg endpoint created successfully.
Waiting for messages from Linux...
```

然后每 10 秒看到：
```
Demo thread: counter = 100, rpmsg messages = 0
Demo thread: counter = 200, rpmsg messages = 0
...
```

## 为什么这个问题之前没有出现？

在裸机 AMP 项目（r5_amp_echo）中：
- 可能向量表在 TCM（地址 0），所以 VBAR 默认值正确
- 或者启动代码中已经设置了 VBAR

在 ThreadX 项目中：
- 向量表在 DDR（0x3ed00000）
- 但没有设置 VBAR
- 导致中断时 CPU 去错误的地址查找向量

## 关键要点

1. **AMP 配置必须设置 VBAR**：当向量表不在地址 0 时
2. **必须在第一个中断之前设置**：在 `setup_timer_interrupt()` 之前
3. **必须在 main 函数开始时设置**：越早越好
4. **这是硬件要求**：不是软件 bug，是 ARM 架构的设计

## 相关文档

- ARM Cortex-R5 Technical Reference Manual, Section 4.3.30 (VBAR)
- Xilinx UG1085: Zynq UltraScale+ Device Technical Reference Manual
- ThreadX User Guide: Interrupt Handling

---

**状态**: ✅ 已修复 - 需要重新编译和测试
**日期**: 2026-02-05
