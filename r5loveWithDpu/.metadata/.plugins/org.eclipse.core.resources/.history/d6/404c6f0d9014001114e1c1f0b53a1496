///*
// * Copyright (c) 2014, Mentor Graphics Corporation
// * All rights reserved.
// *
// * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
// *
// * SPDX-License-Identifier: BSD-3-Clause
// */
//
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xscugic.h"
#include "xil_cache.h"
#include <metal/sys.h>
#include <metal/irq.h>
#include "platform_info.h"
#include <stdarg.h>
#include <stdio.h>

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID

static XScuGic xInterruptController;

/* Interrupt Controller setup */
//static int app_gic_initialize(void)
//{
//	uint32_t status;
//	XScuGic_Config *int_ctrl_config; /* interrupt controller configuration params */
//	uint32_t int_id;
//	uint32_t mask_cpu_id = ((u32)0x1 << XPAR_CPU_ID);
//	uint32_t target_cpu;
//
//	mask_cpu_id |= mask_cpu_id << 8U;
//	mask_cpu_id |= mask_cpu_id << 16U;
//
//	Xil_ExceptionDisable();
//
//	/*
//	 * Initialize the interrupt controller driver
//	 */
//	int_ctrl_config = XScuGic_LookupConfig(INTC_DEVICE_ID);
//	if (NULL == int_ctrl_config) {
//		return XST_FAILURE;
//	}
//
//	status = XScuGic_CfgInitialize(&xInterruptController, int_ctrl_config,
//				       int_ctrl_config->CpuBaseAddress);
//	if (status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}
//
//	/* Only associate interrupt needed to this CPU */
//	for (int_id = 32U; int_id<XSCUGIC_MAX_NUM_INTR_INPUTS;int_id=int_id+4U) {
//		target_cpu = XScuGic_DistReadReg(&xInterruptController,
//						XSCUGIC_SPI_TARGET_OFFSET_CALC(int_id));
//		/* Remove current CPU from interrupt target register */
//		target_cpu &= ~mask_cpu_id;
//		XScuGic_DistWriteReg(&xInterruptController,
//					XSCUGIC_SPI_TARGET_OFFSET_CALC(int_id), target_cpu);
//	}
//	XScuGic_InterruptMaptoCpu(&xInterruptController, XPAR_CPU_ID, IPI_IRQ_VECT_ID);
//
//	/*
//	 * Register the interrupt handler to the hardware interrupt handling
//	 * logic in the ARM processor.
//	 */
//	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
//			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
//			&xInterruptController);
//
//	/* Disable the interrupt before enabling exception to avoid interrupts
//	 * received before exception is enabled.
//	 */
//	XScuGic_Disable(&xInterruptController, IPI_IRQ_VECT_ID);
//
//	Xil_ExceptionEnable();
//
//	/* Connect Interrupt ID with ISR */
//	XScuGic_Connect(&xInterruptController, IPI_IRQ_VECT_ID,
//			(Xil_ExceptionHandler)metal_xlnx_irq_isr,
//			(void *)IPI_IRQ_VECT_ID);
//
//	return 0;
//}
/* 引入在 threadx_amp_demo.c 中已经定义好的中断控制器 */
extern XScuGic InterruptController;

static int app_gic_initialize(void)
{
    /* 此时 GIC 已经被 ThreadX 的定时器初始化好了。
     * 我们只需要把 OpenAMP 的 IPI 中断挂载上去并使能即可，绝对不要再初始化 GIC！ */
    XScuGic_Connect(&InterruptController, IPI_IRQ_VECT_ID,
                    (Xil_ExceptionHandler)metal_xlnx_irq_isr,
                    (void *)IPI_IRQ_VECT_ID);

    XScuGic_Enable(&InterruptController, IPI_IRQ_VECT_ID);

    return 0;
}


static void system_metal_logger(enum metal_log_level level,
			   const char *format, ...)
{
	char msg[1024];
	va_list args;
	static const char *level_strs[] = {
		"metal: emergency: ",
		"metal: alert:     ",
		"metal: critical:  ",
		"metal: error:     ",
		"metal: warning:   ",
		"metal: notice:    ",
		"metal: info:      ",
		"metal: debug:     ",
	};

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	if (level <= METAL_LOG_EMERGENCY || level > METAL_LOG_DEBUG)
		level = METAL_LOG_EMERGENCY;

	xil_printf("%s", level_strs[level]);
	xil_printf("%s", msg);
}


/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	int ret;
	struct metal_init_params metal_param = {
		.log_handler = system_metal_logger,
		.log_level = METAL_LOG_INFO,
	};

	/* Low level abstraction layer for openamp initialization */
	metal_init(&metal_param);

	/* configure the global interrupt controller */
	app_gic_initialize();

	/* Initialize metal Xilinx IRQ controller */
	ret = metal_xlnx_irq_init();
	if (ret) {
		xil_printf("%s: Xilinx metal IRQ controller init failed.\n",
			__func__);
	}

	return ret;
}

void cleanup_system()
{
	metal_finish();

	Xil_DCacheDisable();
	Xil_ICacheDisable();
	Xil_DCacheInvalidate();
	Xil_ICacheInvalidate();
}
