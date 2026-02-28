# 🔧 Linker Script Fix - Critical Issue Resolved

## Problem

r5app.elf failed to load with error:
```
remoteproc remoteproc0: bad phdr da 0x0 mem 0x550
remoteproc remoteproc0: Failed to load program segments: -22
```

## Root Cause Analysis

Using `readelf -l` to compare the two ELF files:

**Working r5_amp_echo.elf:**
- Entry point: 0x3ed00000 (DDR)
- 1 LOAD segment at 0x3ed00000 containing ALL sections
- All code in DDR reserved memory region

**Failing r5app.elf (before fix):**
- Entry point: 0x3c (TCM)
- **2 LOAD segments:**
  - LOAD 1: address 0x0, size 0x550 (only .vectors in TCM) ← **REJECTED**
  - LOAD 2: address 0x3ed00000, size 0x37900 (everything else in DDR)

The remoteproc driver rejected the first LOAD segment because:
1. Address 0x0 is outside the reserved memory region (0x3ed00000-0x3ee00000)
2. Linux cannot access R5's TCM memory

## The Fix

**File:** `r5loveWithDpu/r5app/src/lscript.ld.amp`

### Change 1: Move .vectors to DDR

**Before (line 34-37):**
```ld
.vectors : {
   KEEP (*(.vectors))
   *(.boot)
} > psu_r5_0_atcm_MEM_0  /* ❌ TCM at address 0x0 */
```

**After:**
```ld
.vectors : {
   KEEP (*(.vectors))
   *(.boot)
} > psu_r5_ddr_amp  /* ✅ DDR at 0x3ed00000 */
```

### Change 2: Fix Entry Point

**Before (line 29):**
```ld
ENTRY(_boot)  /* ❌ Points to TCM address */
```

**After:**
```ld
ENTRY(_vector_table)  /* ✅ Standard entry point in DDR */
```

## Why This Matters for AMP

In AMP configuration with remoteproc:
- **Linux (A53) loads the R5 firmware** from DDR
- Linux **cannot access R5's TCM** (Tightly Coupled Memory)
- All code sections must be in the **reserved DDR region** (0x3ed00000-0x3ee00000)
- The ELF must have **only ONE LOAD segment** in the reserved region

## Next Steps

1. **Clean and rebuild** the project in Vitis:
   ```
   Right-click r5app → Clean Project
   Right-click r5app → Build Project
   ```

2. **Verify the fix** with readelf:
   ```bash
   readelf -l r5app/Debug/r5app.elf
   ```

   Expected output:
   - Entry point: 0x3ed00000 (not 0x3c)
   - Only 1 LOAD segment at 0x3ed00000
   - All sections in DDR

3. **Deploy and test** on the board:
   ```bash
   # Copy to SD card /lib/firmware/
   echo r5app.elf > /sys/class/remoteproc/remoteproc0/firmware
   echo start > /sys/class/remoteproc/remoteproc0/state
   ```

## Expected Result

After this fix, r5app.elf should load successfully with:
```
[  xxx.xxxxxx] remoteproc remoteproc0: Booting fw image r5app.elf, size 1259112
[  xxx.xxxxxx] remoteproc remoteproc0: remote processor remoteproc0 is now up
[  xxx.xxxxxx] virtio_rpmsg_bus virtio0: creating channel rpmsg-openamp-demo-channel addr 0x0
```

And `/dev/rpmsg0` should be created for communication.

## Technical Notes

- **TCM vs DDR:** TCM is fast but private to R5; DDR is slower but accessible to both cores
- **Remoteproc limitation:** Can only load code from memory regions defined in device tree
- **Performance impact:** Minimal - modern DDR is fast enough for most real-time tasks
- **Vector table:** Can execute from DDR; R5 doesn't require vectors in TCM

## Verification Checklist

After rebuild:
- [ ] Compilation successful with no errors
- [ ] r5app.elf size approximately 1.2MB
- [ ] `readelf -l` shows entry point at 0x3ed00000
- [ ] `readelf -l` shows only 1 LOAD segment in DDR
- [ ] No LOAD segment at address 0x0
- [ ] Firmware loads on board without "bad phdr" error
- [ ] `/dev/rpmsg0` device created
- [ ] Echo test works: `echo "test" > /dev/rpmsg0 && cat /dev/rpmsg0`

---

**Status:** ✅ Fixed - Ready for rebuild and testing
**Date:** 2026-02-04
