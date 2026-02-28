# ThreadX + AMP 项目配置指南（使用 amp_plat）

## 问题原因

你的 `design_1_wrapperf` platform 没有正确配置 OpenAMP 和 Libmetal 库，导致编译失败。

## 解决方案：使用已配置好的 amp_plat

### 方法 1：在 Vitis IDE 中手动配置

#### 1. 修改编译器包含路径

右键 `r5app` -> Properties -> C/C++ Build -> Settings -> ARM R5 gcc compiler -> Directories

**删除现有的路径，添加**：
```
D:\files\VitisProject\demoAmp\amp_plat\export\amp_plat\sw\amp_plat\standalone_domain\bspinclude\include
```

#### 2. 修改链接器库路径

ARM R5 gcc linker -> Libraries

**Library search path (-L)**：
```
D:\files\VitisProject\demoAmp\amp_plat\psu_cortexr5_0\standalone_domain\bsp\psu_cortexr5_0\lib
```

**Libraries (-l)**（按顺序添加）：
```
open_amp
metal
xil
gcc
c
```

#### 3. 修改 Linker Script

ARM R5 gcc linker -> General -> Script file:
```
../src/lscript.ld.amp
```

#### 4. 清理并编译

```
右键 r5app -> Clean Project
右键 r5app -> Build Project
```

### 方法 2：直接使用 r5_amp_echo 的配置

如果上述方法还是有问题，最简单的方法是：

1. **复制 r5_amp_echo 项目**
2. **将 ThreadX 相关文件添加进去**

#### 步骤：

1. 在 Vitis 中，右键 `r5_amp_echo` -> Copy
2. 粘贴并重命名为 `r5_threadx_amp`
3. 将以下文件复制到新项目的 src 目录：
   - `r5loveWithDpu/r5app/src/threadx/` 目录（整个 ThreadX 源码）
   - `r5loveWithDpu/r5app/src/helloworld.c` 中的 ThreadX 初始化代码

4. 修改项目配置：
   - 添加 ThreadX 包含路径
   - 添加 `-DTX_INCLUDE_USER_DEFINE_FILE` 编译选项

## 方法 3：重新创建 Platform（如果有时间）

如果你想彻底解决问题，可以重新创建一个正确的 platform：

### 在 Vitis 中创建新 Platform

1. File -> New -> Platform Project
2. 命名为 `threadx_amp_plat`
3. 选择你的 XSA 文件
4. 创建 Standalone domain for psu_cortexr5_0

### 配置 BSP

1. 右键 platform -> Board Support Package Settings
2. 在 Standalone domain 中，勾选：
   - ✅ openamp
   - ✅ libmetal
   - ✅ threadx
   - ✅ xilpm

3. 点击 OK，等待 BSP 生成

### 创建应用

1. File -> New -> Application Project
2. 选择刚创建的 `threadx_amp_plat`
3. 创建应用，然后复制源文件

## 快速验证：检查库是否存在

在命令行中运行：

```bash
# 检查 amp_plat 中的库
ls -la D:\files\VitisProject\demoAmp\amp_plat\psu_cortexr5_0\standalone_domain\bsp\psu_cortexr5_0\lib\

# 应该看到：
# libopen_amp.a
# libmetal.a
# libxil.a
```

## 推荐方案

**我强烈推荐使用方法 1**，因为：
1. ✅ `amp_plat` 已经验证可用
2. ✅ 所有库都已正确编译
3. ✅ 只需修改项目配置，不需要重新生成 BSP
4. ✅ 最快速，最可靠

## 编译成功的标志

当配置正确后，编译应该：
- ✅ 找到所有头文件（openamp/open_amp.h, metal/sys.h）
- ✅ 链接所有库（libopen_amp.a, libmetal.a）
- ✅ 生成 r5app.elf 文件（大小约 850KB）

## 如果还是失败

如果上述方法都不行，请提供：
1. 完整的编译错误日志
2. 项目配置截图
3. BSP 配置信息

我会帮你进一步诊断问题。
