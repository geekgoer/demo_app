# 🔍 ThreadX 不运行问题诊断清单

## 当前状态
- ✅ r5app.elf 编译成功
- ✅ remoteproc 可以加载固件
- ❌ `_tx_timer_system_clock` 始终为 0
- ❓ R5 是否真的在运行？

## 🎯 诊断步骤

### 步骤 1: 检查 R5 状态

在 Linux 串口执行：

```bash
# 查看 remoteproc 状态
cat /sys/class/remoteproc/remoteproc0/state

# 查看内核日志（最重要！）
dmesg | tail -50

# 查看固件名称
cat /sys/class/remoteproc/remoteproc0/firmware

# 查看 rpmsg 设备
ls -la /dev/rpmsg*
ls -la /sys/class/rpmsg/
```

**预期输出**：
- state 应该是 `running`
- dmesg 应该显示 "remote processor remoteproc0 is now up"
- 如果有错误，dmesg 会显示具体原因

### 步骤 2: 检查串口输出

**关键问题：你有连接 R5 的串口吗？**

Zynq UltraScale+ 有多个 UART：
- **UART0**: 通常连接到 A53（Linux）
- **UART1**: 通常连接到 R5

**你需要**：
1. 打开两个串口终端（如 MobaXterm、PuTTY）
2. 一个连接 UART0（Linux）
3. 另一个连接 UART1（R5）

**R5 串口应该输出**：
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
```

**如果 R5 串口没有输出**：
- R5 可能根本没有启动
- 或者 UART 配置不对

### 步骤 3: 使用 XSCT 深度检查

```tcl
# 连接
connect
targets
ta 7

# 检查 R5 是否真的在运行
stop
rrd pc
rrd cpsr

# PC 应该在 0x3ed00000 - 0x3ee00000 范围内
# 如果 PC 在其他地方（如 0x0, 0x962c），说明崩溃了

# 检查 VBAR 是否设置成功（通过读取向量表验证）
mrd 0x3ed00000 16

# 应该看到向量表的指令（如 ldr pc, [pc, #24]）

# 检查 ThreadX 是否初始化
mrd 0x3ed218d8  # _tx_timer_system_clock

# 检查栈指针
rrd sp

# 继续运行
con

# 等待 5 秒后再次检查
stop
rrd pc
mrd 0x3ed218d8
```

### 步骤 4: 检查 ELF 文件

在 Windows 上执行：

```bash
# 检查入口点和程序头
readelf -l D:/files/VitisProject/demoAmp/r5loveWithDpu/r5app/Debug/r5app.elf

# 检查 _vector_table 符号
readelf -s D:/files/VitisProject/demoAmp/r5loveWithDpu/r5app/Debug/r5app.elf | grep _vector_table

# 检查 _tx_timer_system_clock 符号
readelf -s D:/files/VitisProject/demoAmp/r5loveWithDpu/r5app/Debug/r5app.elf | grep _tx_timer_system_clock
```

**预期**：
- Entry point: 0x3ed00000
- _vector_table 地址: 0x3ed00000
- _tx_timer_system_clock 地址: 0x3ed218d8

---

## 🔧 可能的问题和解决方案

### 问题 A: R5 根本没有启动

**症状**：
- dmesg 显示错误
- PC 停在 0x0 或其他奇怪的地址
- 没有串口输出

**可能原因**：
1. ELF 文件有问题
2. 内存地址冲突
3. PMU 配置问题

**解决方案**：
- 对比 r5_amp_echo.elf（工作的）和 r5app.elf
- 检查 linker script 是否正确

### 问题 B: R5 启动但立即崩溃

**症状**：
- PC 在 Undefined Mode (CPSR = 0x1b)
- PC 跑飞到 TCM 或其他区域

**可能原因**：
1. VBAR 设置失败
2. 向量表损坏
3. 栈溢出

**解决方案**：
- 检查 VBAR 设置代码
- 增加栈大小

### 问题 C: R5 运行但 ThreadX 没有启动

**症状**：
- PC 在正常范围
- 但 `_tx_timer_system_clock` 为 0

**可能原因**：
1. 定时器中断没有触发
2. `tx_kernel_enter()` 没有被调用
3. ThreadX 初始化失败

**解决方案**：
- 添加更多调试输出
- 检查定时器配置

### 问题 D: 定时器中断没有工作

**症状**：
- R5 运行正常
- 但定时器不触发

**可能原因**：
1. VBAR 设置不正确
2. 中断控制器配置错误
3. TTC 定时器配置错误

**解决方案**：
- 验证 VBAR 值
- 检查 GIC 配置

---

## 📋 请提供以下信息

为了帮你诊断，我需要：

### 1. Linux 串口输出
```bash
dmesg | grep -E "remoteproc|rpmsg|virtio" | tail -30
```

### 2. R5 串口输出
- 你有连接 UART1 吗？
- 如果有，R5 串口显示什么？
- 如果没有，请连接 UART1

### 3. XSCT 详细信息
```tcl
connect
ta 7
stop
rrd pc
rrd cpsr
rrd sp
mrd 0x3ed00000 16
con
```

### 4. ELF 文件信息
```bash
readelf -l r5app.elf | head -20
readelf -s r5app.elf | grep -E "_vector_table|_tx_timer_system_clock|main"
```

---

## 🎯 快速测试：对比工作的版本

让我们先确认 r5_amp_echo 是否还能工作：

```bash
# 在 Linux 上
echo stop > /sys/class/remoteproc/remoteproc0/state
echo r5_amp_echo.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state

# 检查
dmesg | tail -10
ls -la /dev/rpmsg0

# 测试
echo "test" > /dev/rpmsg0
cat /dev/rpmsg0
```

如果 r5_amp_echo 还能工作，说明硬件和 Linux 配置没问题，问题在 r5app.elf。

---

**请先执行这些诊断步骤，然后把输出发给我！**
