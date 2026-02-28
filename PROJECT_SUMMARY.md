# 🎉 AMP 项目完成总结

## 项目背景

**目标**：在 Zynq UltraScale+ MPSoC 上实现 AMP（非对称多处理）系统
- **R5 核心**：运行 ThreadX RTOS
- **A53 核心**：运行 Linux
- **通信方式**：OpenAMP + RPMsg
- **最终目标**：通过 R5 控制 DPU 进行 ResNet50 推理，研究不同调度策略的性能

## ✅ 已完成的工作

### 1. 裸机 AMP 验证（r5_amp_echo 项目）

**问题诊断与解决**：
- ❌ 初始问题：remoteproc 设备未创建
- ❌ 地址不匹配：共享内存地址配置错误
- ❌ Namespace announcement 未启用

**关键修复**：
1. ✅ 修复 `system-user.dtsi` 中的 `#address-cells` 和 `#size-cells` 不一致问题
2. ✅ 修改 `platform_info.c` 中的共享内存地址：`0x3ED40000` → `0x3FD00000`
3. ✅ 启用 namespace announcement：`VIRTIO_RPMSG_F_NS = 1`
4. ✅ 启用详细日志输出（修改 `helper.c`）

**验证结果**：
```bash
root@petalinux:~# echo "Hello from A53!" > /dev/rpmsg0
root@petalinux:~# cat /dev/rpmsg0
Hello from A53!
```
✅ **RPMsg 通信成功！**

### 2. ThreadX + AMP 集成（r5loveWithDpu 项目）

**完成的工作**：
1. ✅ 复制所有 AMP 相关文件到 ThreadX 项目
2. ✅ 创建 AMP 兼容的 linker script (`lscript.ld.amp`)
3. ✅ 创建 ThreadX + AMP 示例应用 (`threadx_amp_demo.c`)
4. ✅ 编写详细的集成指南和快速入门文档

**项目结构**：
```
r5loveWithDpu/
├── r5app/src/
│   ├── threadx_amp_demo.c          # ThreadX + AMP 主应用
│   ├── lscript.ld.amp              # AMP linker script
│   ├── rsc_table.c/h               # Resource table
│   ├── platform_info.c/h           # OpenAMP 平台初始化
│   ├── helper.c                    # 系统初始化
│   ├── zynqmp_r5_a53_rproc.c       # Remoteproc 操作
│   └── rpmsg-echo.c/h              # RPMsg echo（可选）
├── INTEGRATION_GUIDE.md            # 详细集成指南
└── QUICK_START.md                  # 快速入门指南
```

## 🔑 关键技术点

### 1. 内存映射配置

| 区域 | 地址 | 大小 | 用途 |
|------|------|------|------|
| R5 TCM A | 0x00000000 | 64KB | 向量表、启动代码 |
| R5 DDR | 0x3ED00000 | 16MB | 主程序、数据、堆栈 |
| VRing0 | 0x3FD00000 | 16KB | RPMsg TX ring |
| VRing1 | 0x3FD04000 | 16KB | RPMsg RX ring |
| Buffer | 0x3FD08000 | 1MB | 共享缓冲区 |

### 2. Device Tree 配置

**关键节点**：
```dts
zynqmp_r5_rproc: zynqmp_r5_rproc@0 {
    compatible = "xlnx,zynqmp-r5-remoteproc-1.0";
    #address-cells = <2>;  // 必须与 reserved-memory 一致
    #size-cells = <2>;
    core_conf = "split";

    r5_0: r5@0 {
        memory-region = <&rproc_0_reserved>,
                       <&rpu0vdev0vring0>,
                       <&rpu0vdev0vring1>,
                       <&rpu0vdev0buffer>;
        pnode-id = <0x7>;
        tcm-pnode-id = <0xf>, <0x10>;
    };
};
```

### 3. Resource Table 配置

**关键设置**：
```c
#define VIRTIO_RPMSG_F_NS    1  // 必须启用！
#define RING_TX              0x3fd00000
#define RING_RX              0x3fd04000
```

### 4. ThreadX 集成架构

**线程设计**：
- **Demo Thread** (优先级 10)：监控系统状态
- **RPMsg Thread** (优先级 5)：处理 Linux 通信

**中断配置**：
- **TTC Timer**：100Hz，为 ThreadX 提供时钟节拍
- **IPI**：用于 A53-R5 通信

## 📊 测试结果

### 裸机 AMP 测试
```
✅ remoteproc0 设备创建成功
✅ virtio_rpmsg_bus 在线
✅ rpmsg-openamp-demo-channel 创建成功
✅ /dev/rpmsg0 设备存在
✅ Echo 测试通过
```

### 预期的 ThreadX AMP 测试结果
```
✅ ThreadX 内核启动
✅ OpenAMP 初始化成功
✅ RPMsg 端点创建成功
✅ 与 Linux 通信正常
✅ 多线程调度正常
```

## 🚀 下一步工作

### 1. 编译和测试 ThreadX + AMP

在 Vitis IDE 中：
1. 更换 linker script 为 `lscript.ld.amp`
2. 排除 `helloworld.c` 或使用 `threadx_amp_demo.c`
3. 编译项目
4. 部署到板子测试

### 2. 集成 DPU 控制

**设计思路**：
```c
// 添加 DPU 控制线程
void dpu_thread_entry(ULONG thread_input) {
    while (1) {
        // 1. 通过 RPMsg 接收 Linux 的 DPU 任务请求
        // 2. 配置 DPU 寄存器
        // 3. 启动 DPU 推理
        // 4. 等待完成并收集性能数据
        // 5. 通过 RPMsg 返回结果
    }
}
```

**消息协议**：
```c
typedef struct {
    uint32_t cmd;           // START_DPU, STOP_DPU, GET_STATUS
    uint32_t task_id;
    uint32_t input_addr;
    uint32_t output_addr;
    uint32_t model_id;      // ResNet50
    uint32_t priority;      // 任务优先级
} dpu_task_msg_t;
```

### 3. 实验设计

**实验场景**：
1. **全 DPU 模式**：所有算子在 DPU 执行
2. **CPU-DPU 异构模式**：部分算子在 CPU，部分在 DPU
3. **不同调度策略**：
   - 静态调度
   - 动态调度
   - 优先级调度

**性能指标**：
- 任务执行时延
- DPU 利用率
- 通信开销
- 实时性保证

## 📝 重要文件清单

### 配置文件
- `petalinux-files/system-user.dtsi` - Device tree 配置
- `petalinux-files/pm_cfg_obj.c` - PMU 配置
- `r5_amp_echo/src/lscript.ld` - 裸机 linker script
- `r5loveWithDpu/r5app/src/lscript.ld.amp` - ThreadX linker script

### 源代码文件
- `r5_amp_echo/src/rsc_table.c` - Resource table（已修复）
- `r5_amp_echo/src/platform_info.c` - 平台初始化（已修复）
- `r5_amp_echo/src/helper.c` - 系统初始化（已启用日志）
- `r5loveWithDpu/r5app/src/threadx_amp_demo.c` - ThreadX + AMP 应用

### 文档
- `r5loveWithDpu/INTEGRATION_GUIDE.md` - 详细集成指南
- `r5loveWithDpu/QUICK_START.md` - 快速入门
- `PROJECT_SUMMARY.md` - 本文件

## 🎓 学到的经验

### 1. Device Tree 配置陷阱
- `#address-cells` 和 `#size-cells` 必须与 reserved-memory 一致
- 不一致会导致地址解析错误，只读取部分数据

### 2. OpenAMP 配置要点
- `VIRTIO_RPMSG_F_NS` 必须设置为 1，否则 Linux 看不到 rpmsg 通道
- 共享内存地址必须与 device tree 完全一致
- Resource table 必须在 ELF 的 LOAD 段范围内

### 3. 调试技巧
- 启用详细日志输出（修改 `system_metal_logger`）
- 使用 `readelf -l` 检查 ELF 程序头
- 使用 `hexdump` 检查 resource table 内容
- 查看 `/sys/kernel/debug/remoteproc/` 下的调试信息

### 4. PetaLinux 2020.1 特性
- 需要使用 `xlnx,zynqmp-r5-remoteproc-1.0` 作为 compatible 字符串
- 需要添加 `tcm-pnode-id` 属性
- 内核配置需要启用 `CONFIG_RPMSG_CHAR=y`

## 🏆 项目成果

1. ✅ **成功实现裸机 AMP 通信**
   - R5 裸机程序与 Linux 通过 RPMsg 通信
   - Echo 测试完全正常

2. ✅ **完成 ThreadX + AMP 集成准备**
   - 所有文件已准备就绪
   - 详细文档已编写
   - 只需在 Vitis 中编译即可

3. ✅ **为 DPU 实验奠定基础**
   - AMP 通信框架已验证
   - ThreadX 多线程架构已设计
   - 可以开始集成 DPU 控制逻辑

## 📞 技术支持

如果遇到问题，请检查：
1. Vitis 版本：2020.1
2. PetaLinux 版本：2020.1
3. Device tree 配置是否正确
4. Linker script 是否使用 AMP 版本
5. BSP 是否包含 openamp、libmetal、threadx 库

## 🎯 毕设实验建议

### 实验 1：基准测试
- 测试纯 Linux 运行 ResNet50 的性能
- 测试 AMP 模式下的通信开销

### 实验 2：调度策略对比
- 静态调度：预先分配算子
- 动态调度：运行时决策
- 优先级调度：关键路径优先

### 实验 3：实时性分析
- 测量任务响应时间
- 分析抖动（jitter）
- 验证实时性保证

### 实验 4：资源利用率
- DPU 利用率
- CPU 利用率
- 内存带宽

## 🌟 总结

经过详细的问题诊断和修复，我们成功实现了：
1. ✅ Zynq UltraScale+ 上的 AMP 配置
2. ✅ R5 与 Linux 之间的 RPMsg 通信
3. ✅ ThreadX RTOS 与 OpenAMP 的集成框架

现在你可以：
1. 在 Vitis 中编译 ThreadX + AMP 项目
2. 开始集成 DPU 控制逻辑
3. 进行毕设实验和性能分析

**祝你毕设顺利！** 🎓🚀
