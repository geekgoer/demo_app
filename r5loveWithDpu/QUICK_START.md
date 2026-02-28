# ThreadX + AMP 快速配置清单

## ✅ 已完成的准备工作

1. ✅ 所有 AMP 文件已复制到 `r5loveWithDpu/r5app/src/`
2. ✅ 创建了 AMP linker script: `lscript.ld.amp`
3. ✅ 创建了 ThreadX + AMP 应用: `threadx_amp_demo.c`
4. ✅ 修复了关键配置：
   - `VIRTIO_RPMSG_F_NS = 1` (启用 namespace announcement)
   - 共享内存地址 = 0x3FD00000 (与 device tree 一致)
   - 启用了详细日志输出

## 🔧 Vitis 配置步骤（必须执行）

### 1. 更换 Linker Script

在 Vitis IDE 中：
```
右键 r5app -> Properties -> C/C++ Build -> Settings
-> ARM R5 gcc linker -> General -> Script file
改为: ../src/lscript.ld.amp
```

### 2. 配置源文件

**选项 A：使用新应用（推荐）**
```
右键 src/helloworld.c -> Resource Configurations -> Exclude from Build
勾选 Debug 和 Release
```

**选项 B：替换 main 函数**
将 `threadx_amp_demo.c` 的内容复制到 `helloworld.c`

### 3. 确认 BSP 库

右键 BSP 项目 -> Board Support Package Settings
确保已启用：
- ✅ openamp
- ✅ libmetal
- ✅ threadx

### 4. 编译

```
右键 r5app -> Clean Project
右键 r5app -> Build Project
```

## 📋 关键配置对照表

| 配置项 | 裸机 AMP | ThreadX AMP | 说明 |
|--------|----------|-------------|------|
| DDR 起始地址 | 0x3ED00000 | 0x3ED00000 | 相同 |
| DDR 大小 | 16MB | 16MB | 相同 |
| VRing0 地址 | 0x3FD00000 | 0x3FD00000 | 相同 |
| VRing1 地址 | 0x3FD04000 | 0x3FD04000 | 相同 |
| Buffer 地址 | 0x3FD08000 | 0x3FD08000 | 相同 |
| VIRTIO_RPMSG_F_NS | 1 | 1 | 必须启用 |
| 服务名称 | rpmsg-openamp-demo-channel | rpmsg-openamp-demo-channel | 相同 |

## 🚀 部署和测试

### 在板子上（Linux）

```bash
# 1. 复制固件到 /lib/firmware/
cp r5app.elf /lib/firmware/r5_threadx_amp.elf

# 2. 加载并启动
echo r5_threadx_amp.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state

# 3. 验证
dmesg | grep -i rpmsg
# 应该看到: "creating channel rpmsg-openamp-demo-channel"

ls -la /dev/rpmsg*
# 应该看到: /dev/rpmsg0

# 4. 测试通信
echo "Hello ThreadX!" > /dev/rpmsg0
cat /dev/rpmsg0
```

## 🎯 预期输出

### R5 串口输出（如果连接了 UART）
```
===========================================
Starting ThreadX + AMP Demo
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

Demo thread: counter = 100, rpmsg messages = 0
RPMsg: Echoed 10 messages
Demo thread: counter = 200, rpmsg messages = 15
```

### Linux dmesg 输出
```
[   99.419632] remoteproc remoteproc0: powering up r5@0
[   99.460730] remoteproc remoteproc0: Booting fw image r5_threadx_amp.elf, size 818840
[   99.468765]  r5@0: RPU boot from TCM.
[   99.475201] virtio_rpmsg_bus virtio0: rpmsg host is online
[   99.480123] virtio_rpmsg_bus virtio0: creating channel rpmsg-openamp-demo-channel addr 0x400
[   99.484378]  r5@0#vdev0buffer: registered virtio0 (type 7)
[   99.489868] remoteproc remoteproc0: remote processor r5@0 is now up
```

## ⚠️ 常见问题

### 问题：编译时找不到 tx_api.h
**解决**：BSP 中没有启用 threadx 库，需要重新配置 BSP

### 问题：链接时报错 "region psu_r5_ddr_amp overflowed"
**解决**：减少 `DEMO_BYTE_POOL_SIZE` 或栈大小

### 问题：启动后没有 "creating channel" 日志
**解决**：检查 `rsc_table.c` 中 `VIRTIO_RPMSG_F_NS` 是否为 1

### 问题：/dev/rpmsg0 不存在
**解决**：
1. 确认 Linux 内核启用了 `CONFIG_RPMSG_CHAR=y`
2. 尝试 `modprobe rpmsg_char`

## 📊 性能优化建议

### 1. 调整线程优先级
```c
#define RPMSG_PRIORITY    5   // 更高优先级，快速响应
#define DEMO_PRIORITY     10  // 较低优先级
```

### 2. 调整栈大小
```c
#define RPMSG_STACK_SIZE  4096  // 根据实际需求调整
```

### 3. 优化 RPMsg 轮询
```c
// 在 rpmsg_thread_entry 中
while (!shutdown_req) {
    platform_poll(platform_global);
    tx_thread_sleep(1);  // 调整睡眠时间
}
```

## 🔬 下一步：集成 DPU

### 添加 DPU 控制线程

```c
void dpu_thread_entry(ULONG thread_input)
{
    while (1) {
        // 1. 等待来自 Linux 的 DPU 任务请求（通过 RPMsg）
        // 2. 配置 DPU 寄存器
        // 3. 启动 DPU 推理
        // 4. 等待 DPU 完成
        // 5. 将结果发送回 Linux（通过 RPMsg）

        tx_thread_sleep(10);
    }
}
```

### RPMsg 消息格式设计

```c
typedef struct {
    uint32_t cmd;           // 命令类型：START_DPU, STOP_DPU, GET_STATUS
    uint32_t task_id;       // 任务 ID
    uint32_t input_addr;    // 输入数据地址
    uint32_t output_addr;   // 输出数据地址
    uint32_t model_id;      // 模型 ID (ResNet50)
} dpu_task_msg_t;
```

## 📚 相关文件位置

```
r5loveWithDpu/
├── r5app/
│   └── src/
│       ├── threadx_amp_demo.c      # 主应用程序
│       ├── lscript.ld.amp          # Linker script
│       ├── rsc_table.c/h           # Resource table
│       ├── platform_info.c/h       # OpenAMP 平台
│       ├── helper.c                # 系统初始化
│       └── zynqmp_r5_a53_rproc.c   # Remoteproc 操作
├── INTEGRATION_GUIDE.md            # 详细集成指南
└── QUICK_START.md                  # 本文件
```

## ✨ 成功标志

当你看到以下输出时，说明集成成功：

1. ✅ Vitis 编译无错误
2. ✅ Linux dmesg 显示 "creating channel rpmsg-openamp-demo-channel"
3. ✅ `/dev/rpmsg0` 设备存在
4. ✅ `echo "test" > /dev/rpmsg0 && cat /dev/rpmsg0` 返回 "test"

恭喜！你的 ThreadX + AMP 系统已经成功运行！🎉
