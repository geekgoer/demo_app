# 🔧 Vitis 项目配置详细步骤

## 当前问题

编译 r5app 时报错：
```
fatal error: metal/sys.h: No such file or directory
fatal error: openamp/open_amp.h: No such file or directory
```

**原因**：BSP 中没有 openamp 和 libmetal 库

## ✅ 解决方案（推荐）：使用 amp_plat

### 第 1 步：打开项目属性

1. 在 Vitis IDE 的 Project Explorer 中
2. 右键点击 `r5app` 项目
3. 选择 **Properties**

### 第 2 步：修改编译器包含路径

在 Properties 窗口中：

1. 展开 **C/C++ Build**
2. 点击 **Settings**
3. 在右侧，展开 **ARM R5 gcc compiler**
4. 点击 **Directories**

#### 当前的路径（错误的）：
```
D:/files/VitisProject/r5love/design_1_wrapperf/export/design_1_wrapperf/sw/design_1_wrapperf/domain_psu_cortexr5_0/bspinclude/include
```

#### 修改为（正确的）：
```
D:/files/VitisProject/demoAmp/amp_plat/export/amp_plat/sw/amp_plat/standalone_domain/bspinclude/include
```

**操作步骤**：
- 点击现有路径
- 点击 **Edit** 按钮
- 修改路径
- 点击 **OK**

### 第 3 步：修改链接器库路径

在同一个 Settings 窗口中：

1. 展开 **ARM R5 gcc linker**
2. 点击 **Libraries**

#### 3.1 修改 Library search path (-L)

**当前路径**：
```
D:/files/VitisProject/r5love/design_1_wrapperf/...
```

**修改为**：
```
D:/files/VitisProject/demoAmp/amp_plat/psu_cortexr5_0/standalone_domain/bsp/psu_cortexr5_0/lib
```

#### 3.2 添加 Libraries (-l)

确保以下库按顺序存在（如果没有就添加）：
```
open_amp
metal
xil
gcc
c
```

**添加方法**：
- 点击 **Add** 按钮
- 输入库名（不要加 lib 前缀和 .a 后缀）
- 点击 **OK**

### 第 4 步：修改 Linker Script

在同一个 Settings 窗口中：

1. 在 **ARM R5 gcc linker** 下
2. 点击 **General**
3. 找到 **Script file (-T)** 字段

**修改为**：
```
../src/lscript.ld.amp
```

### 第 5 步：应用更改

1. 点击窗口底部的 **Apply and Close**
2. 等待配置保存

### 第 6 步：清理并重新编译

1. 右键点击 `r5app` 项目
2. 选择 **Clean Project**
3. 等待清理完成
4. 右键点击 `r5app` 项目
5. 选择 **Build Project**
6. 等待编译完成

## 📊 预期结果

### 编译成功的输出：

```
Building file: ../src/helper.c
Invoking: ARM R5 gcc compiler
armr5-none-eabi-gcc -DARMR5 -DTX_INCLUDE_USER_DEFINE_FILE ...
Finished building: ../src/helper.c

Building file: ../src/platform_info.c
...
Finished building: ../src/platform_info.c

Building file: ../src/rsc_table.c
...
Finished building: ../src/rsc_table.c

Building file: ../src/threadx_amp_demo.c
...
Finished building: ../src/threadx_amp_demo.c

Building target: r5app.elf
Invoking: ARM R5 gcc linker
...
Finished building target: r5app.elf

Invoking: ARM R5 Print Size
arm-none-eabi-size r5app.elf
   text    data     bss     dec     hex filename
 123456   12345   23456  159257   26e49 r5app.elf
Finished building: r5app.elf.size
```

### 生成的文件：

```
r5loveWithDpu/r5app/Debug/
├── r5app.elf          # 主要的可执行文件（约 850KB）
├── r5app.elf.size     # 大小信息
└── src/
    ├── helper.o
    ├── platform_info.o
    ├── rsc_table.o
    ├── threadx_amp_demo.o
    └── zynqmp_r5_a53_rproc.o
```

## 🔍 验证配置是否正确

### 检查包含路径

在 Vitis 中：
1. 右键 r5app -> Properties
2. C/C++ Build -> Settings -> ARM R5 gcc compiler -> Directories
3. 确认路径指向 `amp_plat`

### 检查库文件是否存在

在 Windows 文件管理器中，检查：
```
D:\files\VitisProject\demoAmp\amp_plat\psu_cortexr5_0\standalone_domain\bsp\psu_cortexr5_0\lib\
```

应该看到：
- ✅ `libopen_amp.a` (约 200KB)
- ✅ `libmetal.a` (约 260KB)
- ✅ `libxil.a` (约 1MB)

## ⚠️ 常见错误

### 错误 1：路径中有空格

如果路径中有空格，需要用引号括起来：
```
"D:/files/Vitis Project/demoAmp/amp_plat/..."
```

### 错误 2：使用了反斜杠

Windows 路径应该使用正斜杠 `/` 而不是反斜杠 `\`：
```
✅ D:/files/VitisProject/demoAmp/...
❌ D:\files\VitisProject\demoAmp\...
```

### 错误 3：库的顺序不对

链接库的顺序很重要，应该是：
```
1. open_amp
2. metal
3. xil
4. gcc
5. c
```

## 🚀 快速配置检查清单

在编译前，确认以下配置：

- [ ] 编译器包含路径指向 `amp_plat/export/.../bspinclude/include`
- [ ] 链接器库路径指向 `amp_plat/.../lib`
- [ ] 库列表包含：open_amp, metal, xil, gcc, c
- [ ] Linker script 设置为 `../src/lscript.ld.amp`
- [ ] 项目已清理（Clean Project）

## 💡 如果还是失败

### 方案 A：检查 amp_plat 是否完整

运行以下命令检查：

```bash
# 在 Git Bash 或命令行中
cd D:/files/VitisProject/demoAmp
find amp_plat -name "libopen_amp.a"
find amp_plat -name "libmetal.a"
```

应该找到这些文件。

### 方案 B：重新生成 amp_plat

如果 amp_plat 不完整：

1. 右键 `amp_plat` -> Clean
2. 右键 `amp_plat` -> Build Project
3. 等待构建完成
4. 然后再编译 r5app

### 方案 C：使用绝对路径

如果相对路径有问题，尝试使用绝对路径：

**编译器包含路径**：
```
D:/files/VitisProject/demoAmp/amp_plat/export/amp_plat/sw/amp_plat/standalone_domain/bspinclude/include
```

**链接器库路径**：
```
D:/files/VitisProject/demoAmp/amp_plat/psu_cortexr5_0/standalone_domain/bsp/psu_cortexr5_0/lib
```

## 📞 需要帮助？

如果按照上述步骤还是失败，请提供：

1. **完整的编译错误日志**（从 Console 窗口复制）
2. **项目配置截图**：
   - C/C++ Build -> Settings -> Directories
   - C/C++ Build -> Settings -> Libraries
3. **检查库文件是否存在**：
   ```bash
   ls -la D:/files/VitisProject/demoAmp/amp_plat/psu_cortexr5_0/standalone_domain/bsp/psu_cortexr5_0/lib/
   ```

---

**记住**：配置正确后，编译应该在 1-2 分钟内完成，不应该有任何错误！
