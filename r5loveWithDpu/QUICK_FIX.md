# 🎯 快速修复指南

## 问题：编译 r5app 时找不到 openamp 和 metal 头文件

## ⚡ 最快的解决方案（5 分钟）

### 在 Vitis IDE 中执行以下操作：

#### 1️⃣ 修改包含路径（2 分钟）

```
右键 r5app -> Properties -> C/C++ Build -> Settings
-> ARM R5 gcc compiler -> Directories
-> 点击现有路径 -> Edit
```

**改为**：
```
D:/files/VitisProject/demoAmp/amp_plat/export/amp_plat/sw/amp_plat/standalone_domain/bspinclude/include
```

#### 2️⃣ 修改库路径（2 分钟）

```
同一窗口 -> ARM R5 gcc linker -> Libraries
-> Library search path (-L) -> Edit
```

**改为**：
```
D:/files/VitisProject/demoAmp/amp_plat/psu_cortexr5_0/standalone_domain/bsp/psu_cortexr5_0/lib
```

**确保 Libraries (-l) 包含**（按顺序）：
```
open_amp
metal
xil
gcc
c
```

#### 3️⃣ 修改 Linker Script（30 秒）

```
同一窗口 -> ARM R5 gcc linker -> General
-> Script file (-T)
```

**改为**：
```
../src/lscript.ld.amp
```

#### 4️⃣ 应用并编译（1 分钟）

```
点击 Apply and Close
右键 r5app -> Clean Project
右键 r5app -> Build Project
```

## ✅ 成功标志

编译成功后，你会看到：
```
Finished building target: r5app.elf
   text    data     bss     dec     hex filename
 xxxxxx   xxxxx   xxxxx  xxxxxx   xxxxx r5app.elf
```

并且在 `r5app/Debug/` 目录下生成 `r5app.elf` 文件（约 850KB）。

## 🔍 如果还是失败

### 检查 1：库文件是否存在

在文件管理器中打开：
```
D:\files\VitisProject\demoAmp\amp_plat\psu_cortexr5_0\standalone_domain\bsp\psu_cortexr5_0\lib\
```

应该看到：
- `libopen_amp.a`
- `libmetal.a`
- `libxil.a`

如果没有这些文件，需要先编译 amp_plat：
```
右键 amp_plat -> Build Project
```

### 检查 2：路径是否正确

确保路径中：
- ✅ 使用正斜杠 `/` 而不是反斜杠 `\`
- ✅ 没有多余的空格
- ✅ 指向正确的目录

### 检查 3：项目是否已清理

```
右键 r5app -> Clean Project
等待清理完成
再次 Build Project
```

## 📋 完整的配置对照表

| 配置项 | 正确的值 |
|--------|----------|
| 编译器包含路径 | `D:/files/VitisProject/demoAmp/amp_plat/export/amp_plat/sw/amp_plat/standalone_domain/bspinclude/include` |
| 链接器库路径 | `D:/files/VitisProject/demoAmp/amp_plat/psu_cortexr5_0/standalone_domain/bsp/psu_cortexr5_0/lib` |
| 链接库（按顺序） | `open_amp`, `metal`, `xil`, `gcc`, `c` |
| Linker Script | `../src/lscript.ld.amp` |

## 🎓 为什么这样做？

- **amp_plat** 是之前成功编译的 platform，已经包含了所有必要的库
- **design_1_wrapperf** 没有正确配置 OpenAMP 和 Libmetal
- 通过指向 amp_plat 的库，我们可以直接使用已编译好的库，避免重新配置 BSP

## 📞 还是不行？

如果按照上述步骤还是失败，请：

1. 截图你的配置（Directories 和 Libraries 页面）
2. 复制完整的编译错误日志
3. 检查 amp_plat 目录是否完整

---

**记住**：只需要修改 3 个配置项，然后 Clean + Build，就应该成功了！
