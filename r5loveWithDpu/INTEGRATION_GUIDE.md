# ThreadX + AMP 集成指南

## 项目概述

本项目将 ThreadX RTOS 与 OpenAMP 集成，实现 R5 核心（运行 ThreadX）与 A53 核心（运行 Linux）之间的通信。

## 已完成的工作

1. ✅ 复制了所有 AMP 相关文件到 r5loveWithDpu/r5app/src/
2. ✅ 创建了 AMP 兼容的 linker script (lscript.ld.amp)
3. ✅ 创建了 ThreadX + AMP 示例应用 (threadx_amp_demo.c)

## 文件清单

### AMP 核心文件
- `rsc_table.c` / `rsc_table.h` - Resource table 定义
- `platform_info.c` / `platform_info.h` - OpenAMP 平台初始化
- `helper.c` - 系统初始化和中断配置
- `zynqmp_r5_a53_rproc.c` - R5-A53 remoteproc 操作
- `rpmsg-echo.c` / `rpmsg-echo.h` - RPMsg echo 示例（可选）

### ThreadX 集成文件
- `threadx_amp_demo.c` - ThreadX + AMP 主应用程序
- `lscript.ld.amp` - AMP 配置的 linker script

## Vitis 配置步骤

### 步骤 1：在 Vitis IDE 中打开项目

1. 打开 Vitis 2020.1
2. 设置 workspace 为 `D:\files\VitisProject\demoAmp\r5loveWithDpu`
3. 项目应该自动加载

### 步骤 2：配置 BSP（Board Support Package）

1. 在 Project Explorer 中找到 BSP 项目（可能是 `design_1_wrapperf` 或类似名称）
2. 右键点击 BSP -> Board Support Package Settings
3. 确保以下库已启用：
   - `openamp` (OpenAMP library)
   - `libmetal` (Libmetal library)
   - `threadx` (ThreadX RTOS)

如果没有这些库，需要添加：
- 在 BSP Settings 中，点击 "Modify BSP Settings"
- 在 "Supported Libraries" 中勾选：
  - openamp
  - libmetal
  - threadx

### 步骤 3：修改项目配置

#### 3.1 更换 Linker Script

1. 右键点击 `r5app` 项目 -> Properties
2. 导航到：C/C++ Build -> Settings -> ARM R5 gcc linker -> General
3. 在 "Script file" 中，将路径改为：
   ```
   ../src/lscript.ld.amp
   ```
4. 点击 Apply

#### 3.2 配置源文件

**方法 A：使用新的 ThreadX + AMP 应用**

1. 在 Project Explorer 中，右键点击 `src/helloworld.c`
2. 选择 "Resource Configurations" -> "Exclude from Build"
3. 勾选 "Debug" 和 "Release"
4. 确保 `threadx_amp_demo.c` 没有被排除

**方法 B：修改现有的 helloworld.c**

如果你想保留现有代码结构，可以将 `threadx_amp_demo.c` 的内容合并到 `helloworld.c` 中。

#### 3.3 添加包含路径

1. 右键点击 `r5app` 项目 -> Properties
2. 导航到：C/C++ Build -> Settings -> ARM R5 gcc compiler -> Directories
3. 确保包含以下路径（应该已经自动添加）：
   - BSP 的 include 路径
   - OpenAMP include 路径
   - Libmetal include 路径

### 步骤 4：编译项目

1. 右键点击 `r5app` 项目
2. 选择 "Clean Project"
3. 选择 "Build Project"
4. 等待编译完成

编译成功后，会在 `r5app/Debug/` 目录生成 `r5app.elf` 文件。

### 步骤 5：部署到板子

1. 将编译好的 `r5app.elf` 复制到 SD 卡的 `/lib/firmware/` 目录
2. 可以重命名为 `r5_threadx_amp.elf` 以区分

## 在 Linux 上测试

### 启动 R5 固件

```bash
# 1. 加载固件
echo r5_threadx_amp.elf > /sys/class/remoteproc/remoteproc0/firmware

# 2. 启动 R5
echo start > /sys/class/remoteproc/remoteproc0/state

# 3. 检查状态
cat /sys/class/remoteproc/remoteproc0/state
dmesg | grep -i rpmsg

# 4. 查看 rpmsg 设备
ls -la /dev/rpmsg*
```

### 测试 RPMsg 通信

```bash
# 发送消息到 R5
echo "Hello from Linux!" > /dev/rpmsg0

# 读取 R5 的回复
cat /dev/rpmsg0 &

# 发送多条消息
for i in {1..10}; do
    echo "Message $i" > /dev/rpmsg0
    sleep 0.1
done

# 停止后台的 cat
killall cat
```

## 内存映射

### R5 内存分配（AMP 模式）

- **TCM A**: 0x00000000 - 0x0000FFFF (64KB) - 向量表和启动代码
- **TCM B**: 0x00020000 - 0x0002FFFF (64KB) - 未使用
- **DDR**: 0x3ED00000 - 0x3EDFFFFF (16MB) - 主程序、数据、堆栈
  - 代码段：从 0x3ED00000 开始
  - 数据段：紧随代码段
  - BSS 段：紧随数据段
  - 堆：动态分配
  - 栈：多个栈（IRQ、FIQ、SVC 等）
  - Resource Table：在 DDR 末尾

### 共享内存区域（用于 RPMsg）

- **VRing0**: 0x3FD00000 (16KB) - TX ring
- **VRing1**: 0x3FD04000 (16KB) - RX ring
- **Buffer**: 0x3FD08000 (1MB) - 共享缓冲区

## 应用程序架构

### ThreadX 线程

1. **Demo Thread** (优先级 10)
   - 每 100ms 运行一次
   - 打印状态信息
   - 监控系统运行

2. **RPMsg Thread** (优先级 5，更高优先级)
   - 初始化 OpenAMP 和 RPMsg
   - 处理来自 Linux 的消息
   - Echo 消息回 Linux

### 中断处理

- **TTC Timer**: 100Hz (10ms) - 为 ThreadX 提供时钟节拍
- **IPI (Inter-Processor Interrupt)**: 用于 A53 和 R5 之间的通信

## 故障排查

### 问题 1：编译错误 - 找不到 openamp 头文件

**解决方案**：
1. 检查 BSP 是否包含 openamp 库
2. 重新生成 BSP：右键 BSP -> Regenerate BSP Sources

### 问题 2：链接错误 - 内存溢出

**解决方案**：
1. 检查 `lscript.ld.amp` 中的内存大小
2. 减少 `DEMO_BYTE_POOL_SIZE` 或栈大小
3. 确保没有使用过大的全局数组

### 问题 3：R5 启动后没有 rpmsg 设备

**解决方案**：
1. 检查 `dmesg | grep remoteproc` 的输出
2. 确认 resource table 地址正确
3. 确认 `VIRTIO_RPMSG_F_NS` 设置为 1

### 问题 4：RPMsg 通信失败

**解决方案**：
1. 检查共享内存地址是否与 device tree 一致
2. 确认 IPI 中断配置正确
3. 查看 R5 的串口输出（如果有）

## 下一步工作

### 集成 DPU 控制

1. 在 ThreadX 中添加 DPU 控制线程
2. 通过 RPMsg 接收来自 Linux 的 DPU 任务请求
3. 在 R5 上调度 DPU 任务
4. 将结果通过 RPMsg 返回给 Linux

### 实验设计

根据你的毕设要求，可以设计以下实验：

1. **全 DPU 模式**：所有算子都在 DPU 上执行
2. **CPU-DPU 异构模式**：部分算子在 CPU，部分在 DPU
3. **不同调度策略**：
   - 静态调度：预先分配算子到 CPU/DPU
   - 动态调度：根据负载动态分配
   - 优先级调度：关键算子优先

### 性能测量

在 ThreadX 中添加性能计数器：
- 任务执行时延
- DPU 利用率
- 通信开销
- 实时性指标

## 参考资料

- Xilinx OpenAMP User Guide (UG1186)
- ThreadX User Guide
- Zynq UltraScale+ MPSoC Technical Reference Manual (UG1085)

## 联系与支持

如果遇到问题，请检查：
1. Vitis 版本是否为 2020.1
2. BSP 配置是否正确
3. Device tree 配置是否与 linker script 一致
