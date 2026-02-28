# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Xilinx Vitis 2020.1 embedded project for Zynq UltraScale+ MPSoC (ZynqMP) implementing Asymmetric Multi-Processing (AMP) with OpenAMP/RPMsg communication between ARM Cortex-R5 and A53 cores.

**Target Platform**: Zynq UltraScale+ MPSoC
**Development Tool**: Xilinx Vitis 2020.1
**Communication Framework**: OpenAMP v1.6 + Libmetal v2.1

## Build System

This project uses **Eclipse CDT managed build** (not standard Makefiles). All builds must be performed through Vitis IDE.

### Building Projects

**Important**: This is an Eclipse-based project. Command-line builds are not supported. All compilation must be done through Vitis IDE:

1. Open Vitis 2020.1 IDE
2. Set workspace to the project directory
3. Right-click project → Clean Project
4. Right-click project → Build Project

Build outputs are generated in `<project>/Debug/` or `<project>/Release/` directories as `.elf` files.

### Testing RPMsg Communication (Linux side)

```bash
# Test script is provided
./test_rpmsg_echo.sh

# Manual testing
echo "Hello from A53!" > /dev/rpmsg0
cat /dev/rpmsg0
```

## Project Structure

### Main Applications

**r5_amp_echo/** - Bare-metal AMP echo application (verified working)
- Main source: `r5_amp_echo/src/rpmsg-echo.c`
- Linker script: `r5_amp_echo/src/lscript.ld`
- Memory: DDR 0x3ED00000-0x3EDFFFFF (16MB)
- Status: ✅ Fully functional, RPMsg communication verified

**r5loveWithDpu/** - ThreadX RTOS + AMP + DPU integration (in development)
- Main application: `r5loveWithDpu/r5app/src/threadx_amp_demo.c`
- AMP linker script: `r5loveWithDpu/r5app/src/lscript.ld.amp`
- ThreadX library: `r5loveWithDpu/libapp/`
- Status: ⏳ Integration complete, awaiting compilation/testing

**amp_plat/** - Hardware platform project
- Hardware spec: `amp_plat/hw/completeHW.xsa`
- BSP location: `amp_plat/psu_cortexr5_0/standalone_domain/bsp/`
- Contains 30+ driver libraries including openamp, libmetal, xscugic, etc.

### Key Configuration Files

**PetaLinux Device Tree**: `petalinux-files/system-user.dtsi`
- Defines R5 remoteproc configuration
- Configures reserved memory regions for AMP
- Sets up IPI mailbox and DPU device nodes

**Resource Table**: `r5_amp_echo/src/rsc_table.c` and `r5loveWithDpu/r5app/src/rsc_table.c`
- Defines virtio device configuration
- Critical: `VIRTIO_RPMSG_F_NS` must be set to 1
- Vring addresses: TX=0x3FD00000, RX=0x3FD04000

**Platform Info**: `platform_info.c` in both projects
- OpenAMP platform initialization
- Shared memory address: 0x3FD00000
- IPI channel configuration

## Memory Map (AMP Configuration)

```
R5 Memory Regions:
├── TCM A: 0x00000000 - 0x0000FFFF (64KB) - Vector table, boot code
├── DDR:   0x3ED00000 - 0x3EDFFFFF (16MB) - Main program, data, heap, stacks
└── Resource table: Located in DDR

Shared Memory (RPMsg):
├── VRing0 (TX): 0x3FD00000 (16KB)
├── VRing1 (RX): 0x3FD04000 (16KB)
└── Buffer:      0x3FD08000 (1MB)
```

## Architecture Notes

### AMP Communication Flow

```
Linux A53 (PetaLinux)          R5 (Bare-metal/ThreadX)
       ↓                                ↓
  /dev/rpmsg0        ←─IPI─→    RPMsg endpoint callback
       ↓                                ↓
  rpmsg_send()       ←─IPI─→    Echo/process message
       ↓                                ↓
  Read response      ←─────     Message sent back
```

### ThreadX Integration Architecture

The `threadx_amp_demo.c` implements a dual-thread design:
- **Demo Thread** (priority 10): System status monitoring
- **RPMsg Thread** (priority 5): Linux communication handling
- **Timer**: TTC configured at 100Hz for ThreadX tick

### BSP Libraries

The platform BSP includes these critical libraries:
- `openamp_v1_6` - OpenAMP framework
- `libmetal_v2_1` - Hardware abstraction layer
- `xscugic_v3_10` - Generic Interrupt Controller driver
- `xttcps_v3_13` - Triple Timer Counter driver
- `xiicps_v3_11` - I2C driver
- Plus 25+ other peripheral drivers

## Critical Configuration Requirements

### Device Tree Requirements

In `system-user.dtsi`, ensure:
- `#address-cells = <2>` and `#size-cells = <2>` must match between remoteproc node and reserved-memory node
- Mismatch causes address parsing errors (only reads partial data)
- `tcm-pnode-id` property is required for PetaLinux 2020.1

### Resource Table Requirements

- `VIRTIO_RPMSG_F_NS` must be 1 (enables namespace announcement)
- Without this, Linux cannot discover the rpmsg channel
- Shared memory addresses must exactly match device tree configuration
- Resource table must be within ELF LOAD segment range

### Linker Script Switching

For AMP mode in ThreadX project:
1. Use `lscript.ld.amp` instead of default `lscript.ld`
2. Configure in Vitis: Project Properties → C/C++ Build → Settings → ARM R5 gcc linker → General → Script file
3. Set to: `../src/lscript.ld.amp`

## Common Issues and Solutions

### Issue: remoteproc device not created
- Check device tree `#address-cells` and `#size-cells` consistency
- Verify reserved memory regions are correctly defined
- Ensure `tcm-pnode-id` is present

### Issue: rpmsg channel not appearing
- Verify `VIRTIO_RPMSG_F_NS = 1` in `rsc_table.c`
- Check shared memory address matches device tree (0x3FD00000)
- Confirm resource table is in ELF LOAD segment: `readelf -l <app>.elf`

### Issue: Communication fails
- Verify IPI channel bitmask (0x01000000 for A53)
- Check vring addresses match device tree
- Enable verbose logging in `helper.c` (modify `system_metal_logger`)

## Debugging Tools

```bash
# Check remoteproc state
cat /sys/class/remoteproc/remoteproc0/state

# View kernel messages
dmesg | grep -i rpmsg
dmesg | grep -i remoteproc

# Check rpmsg devices
ls -la /dev/rpmsg*
ls -la /sys/class/rpmsg/

# Verify ELF structure
readelf -l <app>.elf
readelf -S <app>.elf

# Inspect resource table
hexdump -C <app>.elf | grep -A 20 "resource_table"
```

## File Locations Reference

**Application Source Code**:
- Bare-metal AMP: `r5_amp_echo/src/`
- ThreadX AMP: `r5loveWithDpu/r5app/src/`

**BSP Configuration**:
- Platform BSP: `amp_plat/psu_cortexr5_0/standalone_domain/bsp/psu_cortexr5_0/`
- System MSS: `amp_plat/psu_cortexr5_0/standalone_domain/bsp/system.mss`

**Hardware Files**:
- XSA file: `amp_plat/hw/completeHW.xsa`
- Bitstream: `amp_plat/hw/completeHW.bit`
- PSU init: `amp_plat/hw/psu_init.c/h`

**Documentation**:
- Integration guide: `r5loveWithDpu/INTEGRATION_GUIDE.md`
- Project summary: `PROJECT_SUMMARY.md`
- Troubleshooting: `r5loveWithDpu/TROUBLESHOOTING.md`

## Compiler Flags

Standard flags used across projects:
```
-mcpu=cortex-r5
-mfloat-abi=hard
-mfpu=vfpv3-d16
-c -fmessage-length=0
-MT"$@"
-DARMR5
```

Debug builds use `-O0`, Release builds use `-O2`.

## Important Notes

- Never modify `.cproject` or `.project` files manually - use Vitis IDE
- BSP regeneration may overwrite custom driver modifications
- Always use Vitis 2020.1 - version compatibility is critical
- The project is not under git version control (no .git directory)
- Eclipse metadata in `.metadata/` should not be modified
- Build artifacts are in `Debug/` and `Release/` directories (not tracked)
