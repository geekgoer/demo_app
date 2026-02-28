################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/threadx/ports/tx_initialize_low_level.S \
../src/threadx/ports/tx_thread_context_restore.S \
../src/threadx/ports/tx_thread_context_save.S \
../src/threadx/ports/tx_thread_fiq_context_restore.S \
../src/threadx/ports/tx_thread_fiq_context_save.S \
../src/threadx/ports/tx_thread_fiq_nesting_end.S \
../src/threadx/ports/tx_thread_fiq_nesting_start.S \
../src/threadx/ports/tx_thread_interrupt_control.S \
../src/threadx/ports/tx_thread_interrupt_disable.S \
../src/threadx/ports/tx_thread_interrupt_restore.S \
../src/threadx/ports/tx_thread_irq_nesting_end.S \
../src/threadx/ports/tx_thread_irq_nesting_start.S \
../src/threadx/ports/tx_thread_schedule.S \
../src/threadx/ports/tx_thread_stack_build.S \
../src/threadx/ports/tx_thread_system_return.S \
../src/threadx/ports/tx_thread_vectored_context_save.S \
../src/threadx/ports/tx_timer_interrupt.S 

OBJS += \
./src/threadx/ports/tx_initialize_low_level.o \
./src/threadx/ports/tx_thread_context_restore.o \
./src/threadx/ports/tx_thread_context_save.o \
./src/threadx/ports/tx_thread_fiq_context_restore.o \
./src/threadx/ports/tx_thread_fiq_context_save.o \
./src/threadx/ports/tx_thread_fiq_nesting_end.o \
./src/threadx/ports/tx_thread_fiq_nesting_start.o \
./src/threadx/ports/tx_thread_interrupt_control.o \
./src/threadx/ports/tx_thread_interrupt_disable.o \
./src/threadx/ports/tx_thread_interrupt_restore.o \
./src/threadx/ports/tx_thread_irq_nesting_end.o \
./src/threadx/ports/tx_thread_irq_nesting_start.o \
./src/threadx/ports/tx_thread_schedule.o \
./src/threadx/ports/tx_thread_stack_build.o \
./src/threadx/ports/tx_thread_system_return.o \
./src/threadx/ports/tx_thread_vectored_context_save.o \
./src/threadx/ports/tx_timer_interrupt.o 

S_UPPER_DEPS += \
./src/threadx/ports/tx_initialize_low_level.d \
./src/threadx/ports/tx_thread_context_restore.d \
./src/threadx/ports/tx_thread_context_save.d \
./src/threadx/ports/tx_thread_fiq_context_restore.d \
./src/threadx/ports/tx_thread_fiq_context_save.d \
./src/threadx/ports/tx_thread_fiq_nesting_end.d \
./src/threadx/ports/tx_thread_fiq_nesting_start.d \
./src/threadx/ports/tx_thread_interrupt_control.d \
./src/threadx/ports/tx_thread_interrupt_disable.d \
./src/threadx/ports/tx_thread_interrupt_restore.d \
./src/threadx/ports/tx_thread_irq_nesting_end.d \
./src/threadx/ports/tx_thread_irq_nesting_start.d \
./src/threadx/ports/tx_thread_schedule.d \
./src/threadx/ports/tx_thread_stack_build.d \
./src/threadx/ports/tx_thread_system_return.d \
./src/threadx/ports/tx_thread_vectored_context_save.d \
./src/threadx/ports/tx_timer_interrupt.d 


# Each subdirectory must supply rules for building sources it contributes
src/threadx/ports/%.o: ../src/threadx/ports/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -Wall -O0 -g3 -I"D:\files\VitisProject\demoAmp\r5loveWithDpu\libapp\src\threadx\common" -I"D:\files\VitisProject\r5love\r5app\src\threadx\inc" -I"D:\files\VitisProject\demoAmp\r5loveWithDpu\libapp\src\threadx\ports" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5 -mfloat-abi=hard  -mfpu=vfpv3-d16 -ID:/files/VitisProject/r5love/design_1_wrapperf/export/design_1_wrapperf/sw/design_1_wrapperf/domain_psu_cortexr5_0/bspinclude/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


