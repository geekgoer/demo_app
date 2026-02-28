# 📁 项目文件清单

## 🎯 核心配置文件

### PetaLinux 配置（在 Ubuntu 系统中）
- [ ] `project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi`
  - ✅ 已修复 address-cells 问题
  - ✅ 已添加 tcm-pnode-id
  - ✅ 内存地址配置正确

- [ ] `project-spec/meta-user/recipes-bsp/pmu-firmware/files/pm_cfg_obj.c`
  - ✅ R5_0 和 A53 权限配置
  - ✅ TCM 和 DDR 访问权限

- [ ] `project-spec/meta-user/recipes-kernel/linux/linux-xlnx/bsp.cfg`
  - ✅ CONFIG_REMOTEPROC=y
  - ✅ CONFIG_ZYNQMP_R5_REMOTEPROC=y
  - ✅ CONFIG_RPMSG_CHAR=y
  - ✅ CONFIG_RPMSG_VIRTIO=y

### Vitis 项目文件（Windows 系统）

#### 裸机 AMP 项目 (r5_amp_echo)
- [x] `r5_amp_echo/src/lscript.ld` - Linker script
- [x] `r5_amp_echo/src/rsc_table.c` - Resource table ✅ 已修复
- [x] `r5_amp_echo/src/rsc_table.h`
- [x] `r5_amp_echo/src/platform_info.c` - ✅ 共享内存地址已修复
- [x] `r5_amp_echo/src/platform_info.h`
- [x] `r5_amp_echo/src/helper.c` - ✅ 日志已启用
- [x] `r5_amp_echo/src/zynqmp_r5_a53_rproc.c`
- [x] `r5_amp_echo/src/rpmsg-echo.c`
- [x] `r5_amp_echo/src/rpmsg-echo.h`

#### ThreadX + AMP 项目 (r5loveWithDpu)
- [x] `r5loveWithDpu/r5app/src/threadx_amp_demo.c` - ✅ 新创建
- [x] `r5loveWithDpu/r5app/src/lscript.ld.amp` - ✅ 新创建
- [x] `r5loveWithDpu/r5app/src/rsc_table.c` - ✅ 已复制并修复
- [x] `r5loveWithDpu/r5app/src/rsc_table.h` - ✅ 已复制
- [x] `r5loveWithDpu/r5app/src/platform_info.c` - ✅ 已复制并修复
- [x] `r5loveWithDpu/r5app/src/platform_info.h` - ✅ 已复制
- [x] `r5loveWithDpu/r5app/src/helper.c` - ✅ 已复制并修复
- [x] `r5loveWithDpu/r5app/src/zynqmp_r5_a53_rproc.c` - ✅ 已复制
- [x] `r5loveWithDpu/r5app/src/rpmsg-echo.c` - ✅ 已复制（可选）
- [x] `r5loveWithDpu/r5app/src/rpmsg-echo.h` - ✅ 已复制（可选）
- [x] `r5loveWithDpu/r5app/src/helloworld.c` - 原有文件（需排除或替换）

## 📚 文档文件

- [x] `r5loveWithDpu/INTEGRATION_GUIDE.md` - 详细集成指南
- [x] `r5loveWithDpu/QUICK_START.md` - 快速入门
- [x] `PROJECT_SUMMARY.md` - 项目总结
- [x] `FILE_CHECKLIST.md` - 本文件

## 🔧 关键修改点总结

### 1. rsc_table.c
```c
// 修改前
#define VIRTIO_RPMSG_F_NS    0

// 修改后
#define VIRTIO_RPMSG_F_NS    1  // ✅ 启用 namespace announcement
```

### 2. platform_info.c
```c
// 修改前
#define SHARED_MEM_PA  0x3ED40000UL

// 修改后
#define SHARED_MEM_PA  0x3FD00000UL  // ✅ 与 device tree 一致
```

### 3. helper.c
```c
// 修改前
static void system_metal_logger(enum metal_log_level level,
                               const char *format, ...)
{
    (void)level;
    (void)format;
}

// 修改后
static void system_metal_logger(enum metal_log_level level,
                               const char *format, ...)
{
    // ✅ 完整的日志输出实现
    char msg[1024];
    va_list args;
    // ... 详细实现见文件
}
```

### 4. system-user.dtsi
```dts
// 修改前
zynqmp-rpu@ff9a0000 {
    #address-cells = <1>;  // ❌ 错误
    #size-cells = <1>;     // ❌ 错误
    ...
}

// 修改后
zynqmp_r5_rproc: zynqmp_r5_rproc@0 {
    #address-cells = <2>;  // ✅ 正确
    #size-cells = <2>;     // ✅ 正确
    ...
    r5_0: r5@0 {
        tcm-pnode-id = <0xf>, <0x10>;  // ✅ 新增
    }
}
```

## ✅ 验证清单

### 裸机 AMP 验证
- [x] Vitis 编译成功
- [x] ELF 文件大小约 785KB
- [x] Linux 启动后 remoteproc0 存在
- [x] dmesg 显示 "creating channel rpmsg-openamp-demo-channel"
- [x] /dev/rpmsg0 设备存在
- [x] Echo 测试成功

### ThreadX AMP 验证（待完成）
- [ ] Vitis 编译成功
- [ ] BSP 包含 openamp、libmetal、threadx
- [ ] Linker script 使用 lscript.ld.amp
- [ ] Linux 启动后 remoteproc0 存在
- [ ] dmesg 显示 "creating channel rpmsg-openamp-demo-channel"
- [ ] /dev/rpmsg0 设备存在
- [ ] Echo 测试成功
- [ ] ThreadX 线程正常运行

## 🚀 下一步操作

### 立即执行
1. [ ] 在 Vitis IDE 中打开 r5loveWithDpu 项目
2. [ ] 修改 linker script 为 lscript.ld.amp
3. [ ] 排除 helloworld.c 或使用 threadx_amp_demo.c
4. [ ] 编译项目
5. [ ] 部署到板子测试

### 后续工作
1. [ ] 添加 DPU 控制线程
2. [ ] 设计 RPMsg 消息协议
3. [ ] 实现任务调度策略
4. [ ] 进行性能测试
5. [ ] 收集实验数据

## 📊 文件大小参考

```
r5_amp_echo.elf (裸机):        ~785KB
r5app.elf (ThreadX + AMP):     预计 ~850KB
```

## 🔍 调试命令参考

### 在板子上（Linux）
```bash
# 查看 remoteproc 设备
ls -la /sys/class/remoteproc/

# 查看 remoteproc 状态
cat /sys/class/remoteproc/remoteproc0/state

# 查看内核日志
dmesg | grep -E "remoteproc|rpmsg|virtio"

# 查看 rpmsg 设备
ls -la /dev/rpmsg*
ls -la /sys/class/rpmsg/

# 测试通信
echo "test" > /dev/rpmsg0
cat /dev/rpmsg0
```

### 在 Windows 上（Vitis）
```bash
# 查看 ELF 程序头
readelf -l r5app.elf

# 查看 section 信息
readelf -S r5app.elf

# 查看 resource_table
readelf -x .resource_table r5app.elf

# 查看符号表
readelf -s r5app.elf | grep -i rpmsg
```

## 💾 备份建议

建议备份以下文件：
- [x] 整个 r5_amp_echo 项目（已验证可用）
- [ ] 整个 r5loveWithDpu 项目
- [x] petalinux-files 目录
- [x] 所有 .md 文档文件

## 📞 问题排查流程

如果遇到问题，按以下顺序检查：

1. **编译问题**
   - [ ] BSP 是否包含必要的库
   - [ ] Linker script 路径是否正确
   - [ ] 包含路径是否正确

2. **remoteproc 不存在**
   - [ ] Device tree 是否正确编译
   - [ ] 内核配置是否启用 remoteproc
   - [ ] dmesg 中是否有错误信息

3. **rpmsg 设备不存在**
   - [ ] VIRTIO_RPMSG_F_NS 是否为 1
   - [ ] 共享内存地址是否正确
   - [ ] dmesg 中是否有 "creating channel" 日志

4. **通信失败**
   - [ ] IPI 中断是否配置正确
   - [ ] Resource table 是否在 LOAD 段内
   - [ ] 地址是否与 device tree 一致

## ✨ 成功标志

当所有以下条件满足时，项目成功：
- ✅ Vitis 编译无错误
- ✅ Linux 启动正常
- ✅ remoteproc0 设备存在
- ✅ dmesg 显示 "creating channel"
- ✅ /dev/rpmsg0 存在
- ✅ Echo 测试成功
- ✅ ThreadX 线程正常运行（如果使用 ThreadX）

---

**最后更新**: 2026-02-03
**状态**: 裸机 AMP ✅ 完成 | ThreadX AMP ⏳ 待测试
