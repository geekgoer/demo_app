# ✅ r5app 项目完整配置清单

## 📁 文件清单（已确认）

所有必要的文件都已复制到 `r5loveWithDpu/r5app/src/`：

- ✅ `helper.c`
- ✅ `platform.c` / `platform.h`
- ✅ `platform_config.h`
- ✅ `platform_info.c` / `platform_info.h` ⭐ 刚刚重新复制
- ✅ `rsc_table.c` / `rsc_table.h`
- ✅ `rpmsg-echo.h` ⭐ 刚刚重新复制
- ✅ `threadx_amp_demo.c`
- ✅ `zynqmp_r5_a53_rproc.c`
- ✅ `lscript.ld.amp`
- ✅ `threadx/` 目录（ThreadX 源码）

**注意**：`rpmsg-echo.c` 应该被排除（Exclude from Build），因为它与 `threadx_amp_demo.c` 都有 main 函数。

## 🔧 Vitis 项目配置

### 1. 编译器包含路径

**位置**：右键 r5app → Properties → C/C++ Build → Settings → ARM R5 gcc compiler → Directories

**应该包含以下路径**（按顺序）：

1. `D:/files/VitisProject/demoAmp/amp_plat/export/amp_plat/sw/amp_plat/standalone_domain/bspinclude/include`
   - 用途：OpenAMP 和 Libmetal 头文件

2. `D:/files/VitisProject/demoAmp/r5loveWithDpu/r5app/src`
   - 用途：项目源文件（platform_info.h, rsc_table.h 等）

3. `D:/files/VitisProject/demoAmp/r5loveWithDpu/r5app/src/threadx/inc`
   - 用途：ThreadX 头文件

**删除**：任何包含 `r5love/design_1_wrapperf` 的路径

### 2. 链接器库配置

**位置**：ARM R5 gcc linker → Libraries

#### Libraries (-l)（按顺序）：
```
open_amp    ⭐ 注意：是 open_amp 不是 openamp
metal
xil
gcc
c
```

#### Library search path (-L)（只有一个）：
```
D:/files/VitisProject/demoAmp/amp_plat/psu_cortexr5_0/standalone_domain/bsp/psu_cortexr5_0/lib
```

**删除**：
- ❌ 任何包含 `r5love` 的路径
- ❌ 任何包含 `libapp` 的路径
- ❌ `-lapp` 库

### 3. Linker Script

**位置**：ARM R5 gcc linker → General → Script file (-T)

```
../src/lscript.ld.amp
```

### 4. 编译选项

**位置**：ARM R5 gcc compiler → Miscellaneous

**应该包含**：
```
-DARMR5
-DTX_INCLUDE_USER_DEFINE_FILE
```

### 5. 排除的文件

**位置**：右键 `src/rpmsg-echo.c` → Resource Configurations → Exclude from Build

- ✅ 勾选 Debug
- ✅ 勾选 Release

## 🚀 编译步骤

1. **应用所有配置**
   - 点击 Apply and Close

2. **刷新项目**（可选但推荐）
   - 右键 r5app → Refresh

3. **清理项目**
   - 右键 r5app → Clean Project
   - 等待清理完成

4. **编译项目**
   - 右键 r5app → Build Project
   - 等待编译完成（约 1-2 分钟）

## ✅ 成功标志

编译成功后，你会看到：

```
Finished building target: r5app.elf
Invoking: ARM R5 Print Size
arm-none-eabi-size r5app.elf
   text    data     bss     dec     hex filename
 xxxxxx   xxxxx   xxxxx  xxxxxx   xxxxx r5app.elf
Finished building: r5app.elf.size

Build Finished (took X seconds)
```

并且在 `r5app/Debug/` 目录下生成：
- ✅ `r5app.elf` (约 850KB)
- ✅ `r5app.elf.size`

## ⚠️ 常见错误和解决方案

### 错误 1：`fatal error: platform_info.h: No such file or directory`
**解决**：添加 `src` 目录到包含路径（见上面第 1.2 项）

### 错误 2：`multiple definition of main`
**解决**：排除 `rpmsg-echo.c`（见上面第 5 项）

### 错误 3：`cannot find -lopenamp`
**解决**：库名应该是 `open_amp` 而不是 `openamp`（见上面第 2 项）

### 错误 4：`cannot find -lapp`
**解决**：删除 `-lapp` 库依赖（见上面第 2 项）

### 错误 5：找不到 OpenAMP 头文件
**解决**：确认包含路径指向 `amp_plat`（见上面第 1.1 项）

## 📋 配置验证清单

在编译前，确认以下所有项：

- [ ] 包含路径有 3 个（amp_plat, src, threadx/inc）
- [ ] 没有 `r5love/design_1_wrapperf` 的路径
- [ ] 库名是 `open_amp` 不是 `openamp`
- [ ] 只有一个库路径（指向 amp_plat）
- [ ] 没有 `-lapp` 依赖
- [ ] Linker script 是 `../src/lscript.ld.amp`
- [ ] `rpmsg-echo.c` 已排除
- [ ] 所有源文件都存在（见上面文件清单）

## 🎯 下一步

编译成功后：

1. 将 `r5app.elf` 复制到 SD 卡的 `/lib/firmware/`
2. 在 Linux 中加载：
   ```bash
   echo r5app.elf > /sys/class/remoteproc/remoteproc0/firmware
   echo start > /sys/class/remoteproc/remoteproc0/state
   ```
3. 验证通信：
   ```bash
   echo "Hello ThreadX!" > /dev/rpmsg0
   cat /dev/rpmsg0
   ```

---

**重要提示**：如果按照这个清单配置后还是失败，请提供完整的编译错误日志。
