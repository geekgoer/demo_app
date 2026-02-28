################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/helper.c \
../src/platform.c \
../src/platform_info.c \
../src/rsc_table.c \
../src/threadx_amp_demo.c \
../src/zynqmp_r5_a53_rproc.c 

OBJS += \
./src/helper.o \
./src/platform.o \
./src/platform_info.o \
./src/rsc_table.o \
./src/threadx_amp_demo.o \
./src/zynqmp_r5_a53_rproc.o 

C_DEPS += \
./src/helper.d \
./src/platform.d \
./src/platform_info.d \
./src/rsc_table.d \
./src/threadx_amp_demo.d \
./src/zynqmp_r5_a53_rproc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -DTX_INCLUDE_USER_DEFINE_FILE -Wall -O0 -g3 -I"D:\files\VitisProject\demoAmp\r5loveWithDpu\r5app\src\threadx\inc" -I"D:\files\VitisProject\demoAmp\r5loveWithDpu\r5app\src" -I"D:\files\VitisProject\demoAmp\amp_plat\export\amp_plat\sw\amp_plat\standalone_domain\bspinclude\include" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5 -mfloat-abi=hard  -mfpu=vfpv3-d16 -ID:/files/VitisProject/r5love/design_1_wrapperf/export/design_1_wrapperf/sw/design_1_wrapperf/domain_psu_cortexr5_0/bspinclude/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


