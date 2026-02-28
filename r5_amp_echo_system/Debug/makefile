#
# This file was created by Xilinx Vitis tool.
#


#
# tools
#

RM = rm -f
RMDIR = rm -rf



#
# variables
#

SDCARD_GEN = sdcard_gen
GENERATE_BIF = ::scw::generate_bif

XPFM_PATH = D:/files/VitisProject/demoAmp/amp_plat/export/amp_plat/amp_plat.xpfm
SYS_CONFIG = amp_plat
DOMAINS = standalone_domain
BIF_PATH = D:/files/VitisProject/demoAmp/r5_amp_echo_system/Debug/system.bif

BITSTREAM_PATH = D:/files/VitisProject/demoAmp/r5_amp_echo/_ide/bitstream/completeHW.bit
SD_FILES = --elf D:/files/VitisProject/demoAmp/r5_amp_echo/Debug/r5_amp_echo.elf,psu_cortexr5_0

#
# primary build targets
#

all: package

package:
	-@echo Generating bif file for the system project
	generate_system_bif.bat 3496 ${XPFM_PATH} ${DOMAINS} ${BIF_PATH}
	${SDCARD_GEN} --xpfm ${XPFM_PATH} --sys_config ${SYS_CONFIG} --bif ${BIF_PATH} --bitstream ${BITSTREAM_PATH} ${SD_FILES}

clean:
	$(RMDIR) ./_sds
	${RMDIR} package
	${RM} system.bif

.PHONY: all package clean


