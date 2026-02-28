################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
r5app.elf.size \


# All Target
all: r5app.elf secondary-outputs

# Tool invocations
r5app.elf: $(OBJS) ../src/lscript.ld.amp_fixed $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM R5 gcc linker'
	armr5-none-eabi-gcc -L"D:\files\VitisProject\demoAmp\amp_plat\psu_cortexr5_0\standalone_domain\bsp\psu_cortexr5_0\lib" -L"D:\files\VitisProject\demoAmp\r5loveWithDpu\libapp\Debug" -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16    -Wl,--start-group -lapp -lopen_amp -lmetal -lxil -lgcc -lc -Wl,--end-group -Wl,-T -Wl,../src/lscript.ld.amp_fixed -LD:/files/VitisProject/r5love/design_1_wrapperf/export/design_1_wrapperf/sw/design_1_wrapperf/domain_psu_cortexr5_0/bsplib/lib -o "r5app.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

r5app.elf.size: r5app.elf
	@echo 'Invoking: ARM R5 Print Size'
	armr5-none-eabi-size r5app.elf  |tee "r5app.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(S_UPPER_DEPS)$(C_DEPS)$(ELFSIZE) r5app.elf
	-@echo ' '

secondary-outputs: $(ELFSIZE)

.PHONY: all clean dependents

-include ../makefile.targets
