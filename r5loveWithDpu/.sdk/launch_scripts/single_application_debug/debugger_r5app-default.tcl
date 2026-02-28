connect -url tcp:127.0.0.1:3121
source E:/Xilinx/Vitis/2020.1/scripts/vitis/util/zynqmp_utils.tcl
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -nocase -filter {name =~"APU*"}
enable_a32_mode 0
targets -set -nocase -filter {name =~"RPU*"}
enable_split_mode
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-HS1 210512180081" && level==0 && jtag_device_ctx=="jsn-JTAG-HS1-210512180081-14710093-0"}
fpga -file D:/files/VitisProject/r5loveWithDpu/r5app/_ide/bitstream/design_1_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw D:/files/VitisProject/r5love/design_1_wrapperf/export/design_1_wrapperf/hw/design_1_wrapperf.xsa -mem-ranges [list {0x80000000 0xbfffffff} {0x400000000 0x5ffffffff} {0x1000000000 0x7fffffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
set mode [expr [mrd -value 0xFF5E0200] & 0xf]
targets -set -nocase -filter {name =~ "*A53*#0"}
rst -processor
dow D:/files/VitisProject/r5love/design_1_wrapperf/export/design_1_wrapperf/sw/design_1_wrapperf/boot/fsbl.elf
set bp_2_9_fsbl_bp [bpadd -addr &XFsbl_Exit]
con -block -timeout 60
bpremove $bp_2_9_fsbl_bp
targets -set -nocase -filter {name =~ "*R5*#0"}
rst -processor
dow D:/files/VitisProject/r5loveWithDpu/r5app/Debug/r5app.elf
configparams force-mem-access 0
bpadd -addr &main
