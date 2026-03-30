///******************************************************************************
//*
//* ThreadX + AMP Demo Application for Zynq UltraScale+ MPSoC R5
//*
//* This application demonstrates ThreadX RTOS with RPMsg communication to Linux
//*
//******************************************************************************/
//
//#include <stdio.h>
//#include <string.h>
//#include "platform.h"
//#include "xil_printf.h"
//#include "xil_exception.h"
//#include "xil_cache.h"
//#include "xscugic.h"
//#include "xttcps.h"
//#include "tx_api.h"
//#include "xpseudo_asm.h"  /* For VBAR register access */
//
///* OpenAMP includes */
//#include <openamp/open_amp.h>
//#include <metal/alloc.h>
//#include "platform_info.h"
//#include "rsc_table.h"
//
///* Define ThreadX constants */
//#define DEMO_STACK_SIZE         2048
//#define RPMSG_STACK_SIZE        4096
//#define DEMO_BYTE_POOL_SIZE     32768
//#define DEMO_PRIORITY           10
////#define RPMSG_PRIORITY          5   TODO
//#define RPMSG_PRIORITY          10
//
///* Timer defines */
//#define TTC_DEVICE_ID           XPAR_XTTCPS_0_DEVICE_ID
//#define TTC_TICK_INTR_ID        XPAR_XTTCPS_0_INTR
//#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
//
//#define SHUTDOWN_MSG            0xEF56A55A
//#define LPRINTF(format, ...)    xil_printf(format, ##__VA_ARGS__)
//#define LPERROR(format, ...)    LPRINTF("ERROR: " format, ##__VA_ARGS__)
//
//
//
///* Define the ThreadX object control blocks */
//TX_THREAD               demo_thread;
//TX_THREAD               rpmsg_thread;
//TX_BYTE_POOL            byte_pool_0;
//TX_MUTEX                rpmsg_mutex;
//
///* Define the counters used in the demo application */
//ULONG                   demo_thread_counter;
//ULONG                   rpmsg_msg_counter;
//
///* Diagnostic addresses in R5 reserved DDR tail - reliable from both sides.
// * OCM (0xFFFC0000) is used by PMU firmware, unreliable for diagnostics. */
//#define HEARTBEAT_ADDR  0x3EFF0000
//#define DIAG_ADDR       0x3EFF0004
//volatile uint32_t *heartbeat_ptr = (volatile uint32_t *)HEARTBEAT_ADDR;
//volatile uint32_t *diag_ptr      = (volatile uint32_t *)DIAG_ADDR;
//
///* --- Scenario 1 新增全局变量 --- */
//TX_SEMAPHORE            dpu_sync_sema;
//char                    dpu_result_buf[128];
//
///* Xilinx hardware instances */
////static XScuGic          InterruptController;  TODO
//XScuGic          InterruptController;
//static XTtcPs           TimerInstance;
//
///* OpenAMP variables */
//static struct rpmsg_endpoint lept;
//static int shutdown_req = 0;
//static struct rpmsg_device *rpdev_global = NULL;
//static void *platform_global = NULL;
//
///* Define thread prototypes */
//void    demo_thread_entry(ULONG thread_input);
//void    rpmsg_thread_entry(ULONG thread_input);
//void    setup_timer_interrupt(void);
//void    timer_interrupt_handler(void *CallBackRef);
//
///* ThreadX timer interrupt - external declaration */
//extern void _tx_timer_interrupt(void);
//
///* OpenAMP function prototypes */
//extern int init_system(void);
//extern void cleanup_system(void);
//int rpmsg_app_init(void);
//void rpmsg_app_cleanup(void);
//
///* Define memory pool */
//UCHAR   memory_area[DEMO_BYTE_POOL_SIZE];
//
///**************************************************************************/
///*  RPMsg endpoint callback                                               */
///**************************************************************************/
//static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
//                             uint32_t src, void *priv)
//{
//    (void)priv;
//
//    /* ！！！加入信号捕获雷达，只要收到任何信号，立刻打印 ！！！ */
//    LPRINTF("\r\n[R5 CALLBACK] Received %d bytes from Linux!\r\n", len);
//
//    /* Check for shutdown message */
//    if (len == 4 && (*(unsigned int *)data) == SHUTDOWN_MSG) {
//        // ... 后面的代码保持不变 ...
//        LPRINTF("Shutdown message received.\r\n");
//        shutdown_req = 1;
//        return RPMSG_SUCCESS;
//    }
//
//    /* ！！！极其关键的一步：捕获 Linux 的动态地址，以后发消息就不会丢了 ！！！ */
//    if (ept->dest_addr == RPMSG_ADDR_ANY) {
//        ept->dest_addr = src;
//    }
//
//    rpmsg_msg_counter++;
//
//    if (len > sizeof(dpu_result_buf) - 1) {
//        len = sizeof(dpu_result_buf) - 1;
//    }
//    memcpy(dpu_result_buf, data, len);
//    dpu_result_buf[len] = '\0';
//
//    tx_semaphore_put(&dpu_sync_sema);
//
//    return RPMSG_SUCCESS;
//}
//
//
//static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
//{
//    (void)ept;
//    LPRINTF("Unexpected remote endpoint destroy\r\n");
//    shutdown_req = 1;
//}
//
///**************************************************************************/
///*  Initialize RPMsg application                                          */
///**************************************************************************/
//int rpmsg_app_init(void)
//{
//    int ret;
//
//    *diag_ptr = 0xA0000001;  /* reached rpmsg_app_init */
//
//    /* Initialize platform */
//    ret = platform_init(0, NULL, &platform_global);
//    if (ret) {
//        *diag_ptr = 0xE0000001;  /* platform_init FAILED */
//        return -1;
//    }
//    *diag_ptr = 0xA0000002;  /* platform_init OK */
//
//    rpdev_global = platform_create_rpmsg_vdev(platform_global, 0,
//                                               VIRTIO_DEV_SLAVE,
//                                               NULL, NULL);
//    if (!rpdev_global) {
//        *diag_ptr = 0xE0000002;  /* create_rpmsg_vdev FAILED */
//        return -1;
//    }
//    *diag_ptr = 0xA0000003;  /* create_rpmsg_vdev OK */
//
//    /* Create RPMsg endpoint */
////    ret = rpmsg_create_ept(&lept, rpdev_global, "rpmsg-openamp-demo-channel",
////                           0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb,
////                           rpmsg_service_unbind);
//    /* Create RPMsg endpoint */
////	ret = rpmsg_create_ept(&lept, rpdev_global, "rpmsg-chrdev",  /* <--- 唯一需要改的地方！ */
////						   0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb,
////						   rpmsg_service_unbind);
//    /* Create RPMsg endpoint */
////	ret = rpmsg_create_ept(&lept, rpdev_global, "rpmsg-chrdev",
////						   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,  /* <--- 两个参数全部改成 RPMSG_ADDR_ANY */
////						   rpmsg_endpoint_cb,
////						   rpmsg_service_unbind);
//    /* Create RPMsg endpoint */
////        ret = rpmsg_create_ept(&lept, rpdev_global, "rpmsg-chrdev",
////                               50, RPMSG_ADDR_ANY,    /* <--- 把源地址写成 50！ */
////                               rpmsg_endpoint_cb,
////                               rpmsg_service_unbind);
//    ret = rpmsg_create_ept(&lept, rpdev_global, "rpmsg-raw", /* <--- 暗号改成 rpmsg-raw */
//                               50, RPMSG_ADDR_ANY,
//                               rpmsg_endpoint_cb,
//                               rpmsg_service_unbind);
//    if (ret) {
//        *diag_ptr = 0xE0000003;  /* rpmsg_create_ept FAILED */
//        return -1;
//    }
//    *diag_ptr = 0xA0000004;  /* rpmsg_create_ept OK - endpoint created! */
//
//    LPRINTF("RPMsg endpoint created successfully.\r\n");
//    LPRINTF("Waiting for messages from Linux...\r\n");
//
//    return 0;
//}
//
///**************************************************************************/
///*  Cleanup RPMsg application                                             */
///**************************************************************************/
//void rpmsg_app_cleanup(void)
//{
//    if (rpdev_global) {
//        rpmsg_destroy_ept(&lept);
//        platform_release_rpmsg_vdev(rpdev_global);
//    }
//
//    if (platform_global) {
//        platform_cleanup(platform_global);
//    }
//}
//
///**************************************************************************/
///*  Demo thread entry                                                     */
///**************************************************************************/
//void demo_thread_entry(ULONG thread_input)
//{
//    (void)thread_input;
//    char send_buf[64];
//    int frame_count = 1;
//
//    LPRINTF("Scenario 1: Serial Blocking Baseline Started...\r\n");
//    LPRINTF("Waiting for Linux to send START signal (Please run ./resnet50 now)...\r\n");
//
//    /* 死死阻塞！直到你在 Linux 端敲下 ./resnet50 并发来 START 信号 */
//    tx_semaphore_get(&dpu_sync_sema, TX_WAIT_FOREVER);
//
//    LPRINTF("Linux is Online! Commencing DPU pipeline...\r\n");
//
//    while (frame_count <= 100) {
//        sprintf(send_buf, "INFER_REQ: FRAME_%03d", frame_count);
//
//        ULONG t1 = tx_time_get();
//
//        /* 因为前面已经捕获了 dest_addr，这里的发送将百发百中！ */
//        rpmsg_send(&lept, send_buf, strlen(send_buf) + 1);
//
//        /* 放心死等 DPU 的结果 */
//        tx_semaphore_get(&dpu_sync_sema, TX_WAIT_FOREVER);
//
//        ULONG t2 = tx_time_get();
//
//        LPRINTF("Frame %03d | Latency: %lu ticks | Result: %s\r\n",
//                frame_count, (t2 - t1), dpu_result_buf);
//
//        frame_count++;
//        tx_thread_sleep(5);
//    }
//
//    LPRINTF("Scenario 1 Test Completed.\r\n");
//    while(1) { tx_thread_sleep(100); }
//}
//
///**************************************************************************/
///*  RPMsg thread entry                                                    */
///**************************************************************************/
//void rpmsg_thread_entry(ULONG thread_input)
//{
//    (void)thread_input;
//
//    LPRINTF("RPMsg thread started\r\n");
//
//    /* Initialize RPMsg */
//    if (rpmsg_app_init() != 0) {
//        LPERROR("RPMsg initialization failed!\r\n");
//        return;
//    }
//
//    /* Main RPMsg loop */
//    /* Main RPMsg loop */
//	while (!shutdown_req) {
//		platform_poll(platform_global);
//		tx_thread_relinquish();  /* 安全交出 CPU，不再依赖定时器睡眠！ */
//	}
//
//    LPRINTF("RPMsg thread shutting down...\r\n");
//    rpmsg_app_cleanup();
//}
//
/////**************************************************************************/
/////*  Timer interrupt handler                                               */
/////**************************************************************************/
//void timer_interrupt_handler(void *CallBackRef)
//{
//    XTtcPs *TtcPsInst = (XTtcPs *)CallBackRef;
//    u32 StatusEvent;
//
//    /* Read and clear the interrupt status */
//    StatusEvent = XTtcPs_GetInterruptStatus(TtcPsInst);
//    XTtcPs_ClearInterruptStatus(TtcPsInst, StatusEvent);
//
//    /* Call ThreadX timer interrupt */
//    _tx_timer_interrupt();
//}
//
///**************************************************************************/
///*  Setup timer interrupt                                                 */
///**************************************************************************/
//void setup_timer_interrupt(void)
//{
//    int Status;
//    XTtcPs_Config *Config;
//    XScuGic_Config *IntcConfig;
//    XInterval Interval;  /* Changed from u16 to XInterval (u32) */
//    u8 Prescaler;
//
//    /* Initialize the interrupt controller */
//    IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
//    Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
//                                   IntcConfig->CpuBaseAddress);
//    if (Status != XST_SUCCESS) {
//        xil_printf("Interrupt controller init failed\r\n");
//        return;
//    }
//
//    /* Initialize the timer */
//    Config = XTtcPs_LookupConfig(TTC_DEVICE_ID);
//    Status = XTtcPs_CfgInitialize(&TimerInstance, Config, Config->BaseAddress);
//    if (Status != XST_SUCCESS) {
//        xil_printf("Timer init failed\r\n");
//        return;
//    }
//
//    /* Set timer options */
//    XTtcPs_SetOptions(&TimerInstance, XTTCPS_OPTION_INTERVAL_MODE |
//                                      XTTCPS_OPTION_WAVE_DISABLE);
//
//    /* Calculate timer interval for 10ms (100Hz) */
//    XTtcPs_CalcIntervalFromFreq(&TimerInstance, 100, &Interval, &Prescaler);
//    XTtcPs_SetInterval(&TimerInstance, Interval);
//    XTtcPs_SetPrescaler(&TimerInstance, Prescaler);
//
//    /* Connect timer interrupt */
//    Status = XScuGic_Connect(&InterruptController, TTC_TICK_INTR_ID,
//                            (Xil_ExceptionHandler)timer_interrupt_handler,
//                            (void *)&TimerInstance);
//    if (Status != XST_SUCCESS) {
//        xil_printf("Timer interrupt connect failed\r\n");
//        return;
//    }
//
//    /* Enable timer interrupt */
//    XScuGic_Enable(&InterruptController, TTC_TICK_INTR_ID);
//    XTtcPs_EnableInterrupts(&TimerInstance, XTTCPS_IXR_INTERVAL_MASK);
//
//    /* Register interrupt controller handler */
//    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
//                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
//                                 &InterruptController);
//
//    /* Enable interrupts */
//    Xil_ExceptionEnable();
//
//    /* Start the timer */
//    XTtcPs_Start(&TimerInstance);
//}
//
///**************************************************************************/
///*  Application define function                                           */
///**************************************************************************/
//void tx_application_define(void *first_unused_memory)
//{
//    CHAR *pointer;
//    UINT status;
//
//    /* Create a byte memory pool */
//    status = tx_byte_pool_create(&byte_pool_0, "byte pool 0", memory_area,
//                                 DEMO_BYTE_POOL_SIZE);
//    if (status != TX_SUCCESS) {
//        xil_printf("Failed to create byte pool\r\n");
//        return;
//    }
//
//    /* Create mutex for RPMsg */
//    status = tx_mutex_create(&rpmsg_mutex, "rpmsg mutex", TX_NO_INHERIT);
//    if (status != TX_SUCCESS) {
//        xil_printf("Failed to create mutex\r\n");
//        return;
//    }
//
//    /* Allocate stack for demo thread */
//    status = tx_byte_allocate(&byte_pool_0, (VOID **)&pointer,
//                              DEMO_STACK_SIZE, TX_NO_WAIT);
//    if (status != TX_SUCCESS) {
//        xil_printf("Failed to allocate demo thread stack\r\n");
//        return;
//    }
//
//    /* Create demo thread */
//    status = tx_thread_create(&demo_thread, "Demo Thread", demo_thread_entry, 0,
//                              pointer, DEMO_STACK_SIZE,
//                              DEMO_PRIORITY, DEMO_PRIORITY,
//                              TX_NO_TIME_SLICE, TX_AUTO_START);
//    if (status != TX_SUCCESS) {
//        xil_printf("Failed to create demo thread\r\n");
//        return;
//    }
//    /* --- Scenario 1: 创建用于阻塞等待的信号量 --- */
//	status = tx_semaphore_create(&dpu_sync_sema, "DPU Sync Semaphore", 0);
//	if (status != TX_SUCCESS) {
//		xil_printf("Failed to create semaphore\r\n");
//		return;
//	}
//
//    /* Allocate stack for RPMsg thread */
//    status = tx_byte_allocate(&byte_pool_0, (VOID **)&pointer,
//                              RPMSG_STACK_SIZE, TX_NO_WAIT);
//    if (status != TX_SUCCESS) {
//        xil_printf("Failed to allocate rpmsg thread stack\r\n");
//        return;
//    }
//
//    /* Create RPMsg thread */
//    status = tx_thread_create(&rpmsg_thread, "RPMsg Thread", rpmsg_thread_entry, 0,
//                              pointer, RPMSG_STACK_SIZE,
//                              RPMSG_PRIORITY, RPMSG_PRIORITY,
//                              TX_NO_TIME_SLICE, TX_AUTO_START);
//    if (status != TX_SUCCESS) {
//        xil_printf("Failed to create rpmsg thread\r\n");
//        return;
//    }
//
//    xil_printf("ThreadX application defined successfully\r\n");
//}
//
//
////int main()
////{
////    *heartbeat_ptr = 0xDEADBEE1;
////
////    /* 1. 手工复制中断向量表到 TCM (0x0) */
////    extern u32 _vector_table;
////    volatile u32 *src = (volatile u32 *)&_vector_table;
////    volatile u32 *dst = (volatile u32 *)0x0;
////    for (int i = 0; i < 16; i++) {
////        dst[i] = src[i];
////    }
////    __asm__ volatile("dsb");
////    __asm__ volatile("isb");
////
////    /* 2. 开启 FPU */
////    __asm__ volatile(
////        "mrc p15, 0, r0, c1, c0, 2 \n"
////        "orr r0, r0, #(0xF << 20)  \n"
////        "mcr p15, 0, r0, c1, c0, 2 \n"
////        "isb                       \n"
////        "vmrs r0, fpexc            \n"
////        "orr r0, r0, #(1<<30)      \n"
////        "vmsr fpexc, r0            \n"
////        : : : "r0"
////    );
////
////    /* 3. 初始化平台 (MPU, Cache, UART) */
////    init_platform();
////
////    /* 这里千万不要再有 setup_timer_interrupt(); 了！ */
////
////    *heartbeat_ptr = 0xDEADBEE2;
////
////    /* 4. 启动 ThreadX 调度器 */
////    tx_kernel_enter();
////
////    return 0;
////}
////int main()
////{
////    *heartbeat_ptr = 0xDEADBEE1;
////
////    extern u32 _vector_table;
////
////    /* 1. 安全配置 VBAR 并关闭 SCTLR.V 位 (使用 C 变量传参，绝对安全！) */
////    __asm__ volatile(
////        "mrc p15, 0, r0, c1, c0, 0 \n"
////        "bic r0, r0, #(1<<13)      \n"  /* 清除 V 位 */
////        "mcr p15, 0, r0, c1, c0, 0 \n"
////        "mcr p15, 0, %0, c12, c0, 0\n"  /* 把 _vector_table 的地址写入 VBAR */
////        "isb                       \n"
////        "dsb                       \n"
////        : : "r"((u32)&_vector_table) : "r0"  /* <--- 关键修复：让编译器安全传入地址 */
////    );
////
////    /* 2. 开启 FPU (必须保留，防止 ThreadX 浮点指令死机) */
////    __asm__ volatile(
////        "mrc p15, 0, r0, c1, c0, 2 \n"
////        "orr r0, r0, #(0xF << 20)  \n"
////        "mcr p15, 0, r0, c1, c0, 2 \n"
////        "isb                       \n"
////        "vmrs r0, fpexc            \n"
////        "orr r0, r0, #(1<<30)      \n"
////        "vmsr fpexc, r0            \n"
////        : : : "r0"
////    );
////
////    /* 3. 初始化平台 (开启 MPU, Cache, UART) */
////    init_platform();
////
////    *heartbeat_ptr = 0xDEADBEE2;
////
////    /* 4. 启动 ThreadX 调度器 */
////    tx_kernel_enter();
////
////    return 0;
////}
////int main()
////{
////    *heartbeat_ptr = 0xDEADBEE1;
////
////    /* 1. 【王者回归】：手工复制中断向量表到 TCM (0x0)
////     * 完美解决 Linux 拒载和硬件寻址的矛盾！ */
////    extern u32 _vector_table;
////    volatile u32 *src = (volatile u32 *)&_vector_table;
////    volatile u32 *dst = (volatile u32 *)0x0;
////    for (int i = 0; i < 16; i++) {
////        dst[i] = src[i];
////    }
////    __asm__ volatile("dsb");
////    __asm__ volatile("isb");
////
////    /* 2. 开启 FPU (必须保留，ThreadX 调度强依赖它) */
////    __asm__ volatile(
////        "mrc p15, 0, r0, c1, c0, 2 \n"
////        "orr r0, r0, #(0xF << 20)  \n"
////        "mcr p15, 0, r0, c1, c0, 2 \n"
////        "isb                       \n"
////        "vmrs r0, fpexc            \n"
////        "orr r0, r0, #(1<<30)      \n"
////        "vmsr fpexc, r0            \n"
////        : : : "r0"
////    );
////
////    /* 3. 初始化平台 (开启 MPU, Cache, UART) */
////    init_platform();
////
////    *heartbeat_ptr = 0xDEADBEE2;
////
////    /* 4. 启动 ThreadX 调度器 */
////    tx_kernel_enter();
////
////    return 0;
////}
//int main()
//{
//    *heartbeat_ptr = 0xDEADBEE1;
//
//    extern u32 _vector_table;
//    volatile u32 *src = (volatile u32 *)&_vector_table;
//    volatile u32 *dst = (volatile u32 *)0x0;
//    for (int i = 0; i < 16; i++) {
//        dst[i] = src[i];
//    }
//    __asm__ volatile("dsb");
//    __asm__ volatile("isb");
//
//    __asm__ volatile(
//        "mrc p15, 0, r0, c1, c0, 2 \n"
//        "orr r0, r0, #(0xF << 20)  \n"
//        "mcr p15, 0, r0, c1, c0, 2 \n"
//        "isb                       \n"
//        "vmrs r0, fpexc            \n"
//        "orr r0, r0, #(1<<30)      \n"
//        "vmsr fpexc, r0            \n"
//        : : : "r0"
//    );
//
//    /* 3. 初始化平台 (开启 MPU, Cache, UART) */
//    init_platform();
//
//    /* ！！！王者归来：启动定时器心跳 ！！！*/
//    setup_timer_interrupt();
//
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* 4. 启动 ThreadX 调度器 */
//    tx_kernel_enter();
//
//    return 0;
//}
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "tx_api.h"
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rsc_table.h"

#define DEMO_STACK_SIZE         2048
#define RPMSG_STACK_SIZE        4096
#define DEMO_BYTE_POOL_SIZE     32768
#define DEMO_PRIORITY           10
#define RPMSG_PRIORITY          10
#define SHUTDOWN_MSG            0xEF56A55A
#define LPRINTF(format, ...)    xil_printf(format, ##__VA_ARGS__)

TX_THREAD               demo_thread;
TX_THREAD               rpmsg_thread;
TX_BYTE_POOL            byte_pool_0;
TX_SEMAPHORE            dpu_sync_sema;

char                    dpu_result_buf[128];
volatile uint32_t *heartbeat_ptr = (volatile uint32_t *)0x3EFF0000;

static struct rpmsg_endpoint lept;
static int shutdown_req = 0;
static struct rpmsg_device *rpdev_global = NULL;
static void *platform_global = NULL;

extern int init_system(void);
extern void cleanup_system(void);
UCHAR memory_area[DEMO_BYTE_POOL_SIZE];

/* ========================================================================= */
/* 极高精度硬件秒表 (PMU) 驱动代码 */
/* ========================================================================= */

/* 1. 初始化并启动 PMU 周期计数器 */
void pmu_enable(void) {
    /* 通过 CP15 协处理器，写 PMCR 寄存器：使能 PMU (bit 0) 并重置计数器 (bit 2) */
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 0" :: "r"(1 | 4));
    /* 写 PMCNTENSET 寄存器：单独使能周期计数器 (bit 31) */
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 1" :: "r"(1 << 31));
}

/* 2. 获取当前时钟周期数 */
static inline uint32_t get_cycle_count(void) {
    uint32_t count;
    /* 读取 PMCCNTR 寄存器 */
    __asm__ volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(count));
    return count;
}


/* 替代 tx_thread_sleep 的裸机延时函数 */
void baremetal_delay_ms(int ms) {
    for (volatile int i = 0; i < ms * 20000; i++);
}

static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                             uint32_t src, void *priv)
{
    (void)priv;
    LPRINTF("\r\n[R5 CALLBACK] Received %d bytes from Linux!\r\n", len);

    if (len == 4 && (*(unsigned int *)data) == SHUTDOWN_MSG) {
        shutdown_req = 1; return RPMSG_SUCCESS;
    }

    /* 捕获 Linux 的动态源地址 */
    if (ept->dest_addr == RPMSG_ADDR_ANY) {
        ept->dest_addr = src;
    }

    if (len > sizeof(dpu_result_buf) - 1) len = sizeof(dpu_result_buf) - 1;
    memcpy(dpu_result_buf, data, len);
    dpu_result_buf[len] = '\0';

    tx_semaphore_put(&dpu_sync_sema);
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept) {
    (void)ept; shutdown_req = 1;
}

int rpmsg_app_init(void) {
    platform_init(0, NULL, &platform_global);
    rpdev_global = platform_create_rpmsg_vdev(platform_global, 0, VIRTIO_DEV_SLAVE, NULL, NULL);
    rpmsg_create_ept(&lept, rpdev_global, "rpmsg-raw", 50, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);
    LPRINTF("RPMsg endpoint created successfully.\r\n");
    return 0;
}

//void demo_thread_entry(ULONG thread_input) {
//    (void)thread_input;
//    char send_buf[64];
//    int frame_count = 1;
//
//    LPRINTF("Scenario 1: Serial Blocking Baseline Started...\r\n");
//    LPRINTF("Waiting for Linux to send START signal (Please run ./resnet50 now)...\r\n");
//
//    /* 死等 Linux 第一次发来的 START 信号 */
//    tx_semaphore_get(&dpu_sync_sema, TX_WAIT_FOREVER);
//
//    LPRINTF("Linux is Online! Commencing DPU pipeline...\r\n");
//
//    while (frame_count <= 100) {
//        sprintf(send_buf, "INFER_REQ: FRAME_%03d", frame_count);
//
//        /* 发送推理指令 */
//        rpmsg_send(&lept, send_buf, strlen(send_buf) + 1);
//
//        /* 死等推理结果返回 */
//        tx_semaphore_get(&dpu_sync_sema, TX_WAIT_FOREVER);
//
//        /* 打印结果 */
//        LPRINTF("Frame %03d | Result: %s\r\n", frame_count, dpu_result_buf);
//
//        frame_count++;
//        baremetal_delay_ms(50);
//    }
//
//    LPRINTF("Scenario 1 Test Completed.\r\n");
//    while(1) { baremetal_delay_ms(1000); }
//}

void demo_thread_entry(ULONG thread_input) {
    (void)thread_input;
    char send_buf[64];
    int frame_count = 1;

    LPRINTF("Scenario 1: Serial Blocking Baseline Started...\r\n");
    LPRINTF("Waiting for Linux to send START signal (Please run ./resnet50 now)...\r\n");

    /* 死等 Linux 第一次发来的 START 信号 */
    tx_semaphore_get(&dpu_sync_sema, TX_WAIT_FOREVER);

    LPRINTF("Linux is Online! Commencing DPU pipeline...\r\n");

    while (frame_count <= 100) {
        sprintf(send_buf, "INFER_REQ: FRAME_%03d", frame_count);

        /* 1. 按下物理秒表，记录起始周期数 */
        uint32_t t1_cycles = get_cycle_count();

        /* 发送推理指令 */
        rpmsg_send(&lept, send_buf, strlen(send_buf) + 1);

        /* 死等推理结果返回 */
        tx_semaphore_get(&dpu_sync_sema, TX_WAIT_FOREVER);

        /* 2. 再次按下物理秒表，记录结束周期数 */
        uint32_t t2_cycles = get_cycle_count();

        /* 3. 计算耗时：因为 R5 是 500MHz，所以 1 毫秒 = 500,000 个周期 */
        uint32_t delta_cycles = t2_cycles - t1_cycles;
        float latency_ms = (float)delta_cycles / 500000.0f;

        /* 打印带有精准毫秒级时延的结果 */
        /* 注意：xil_printf 不支持直接打 float，所以强转成整数打出 毫秒 和 微秒 部分 */
        int ms_int = (int)latency_ms;
        int us_int = (int)((latency_ms - ms_int) * 1000);
        LPRINTF("Frame %03d | Latency: %d.%03d ms | Result: %s\r\n",
                frame_count, ms_int, us_int, dpu_result_buf);

        frame_count++;
        baremetal_delay_ms(50);
    }

    LPRINTF("Scenario 1 Test Completed.\r\n");
    while(1) { baremetal_delay_ms(1000); }
}


void rpmsg_thread_entry(ULONG thread_input) {
    (void)thread_input;
    LPRINTF("RPMsg thread started\r\n");
    if (rpmsg_app_init() != 0) return;

    while (!shutdown_req) {
        platform_poll(platform_global);
        tx_thread_relinquish();
    }
}

void tx_application_define(void *first_unused_memory) {
    CHAR *pointer;
    tx_byte_pool_create(&byte_pool_0, "byte pool 0", memory_area, DEMO_BYTE_POOL_SIZE);

    tx_byte_allocate(&byte_pool_0, (VOID **)&pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    tx_thread_create(&demo_thread, "Demo Thread", demo_thread_entry, 0, pointer, DEMO_STACK_SIZE, DEMO_PRIORITY, DEMO_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);

    tx_semaphore_create(&dpu_sync_sema, "DPU Sync Semaphore", 0);

    tx_byte_allocate(&byte_pool_0, (VOID **)&pointer, RPMSG_STACK_SIZE, TX_NO_WAIT);
    tx_thread_create(&rpmsg_thread, "RPMsg Thread", rpmsg_thread_entry, 0, pointer, RPMSG_STACK_SIZE, RPMSG_PRIORITY, RPMSG_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
}

int main() {
    *heartbeat_ptr = 0xDEADBEE1;

    extern u32 _vector_table;
    volatile u32 *src = (volatile u32 *)&_vector_table;
    volatile u32 *dst = (volatile u32 *)0x0;
    for (int i = 0; i < 16; i++) dst[i] = src[i];
    __asm__ volatile("dsb"); __asm__ volatile("isb");

    __asm__ volatile(
        "mrc p15, 0, r0, c1, c0, 2 \n"
        "orr r0, r0, #(0xF << 20)  \n"
        "mcr p15, 0, r0, c1, c0, 2 \n"
        "isb                       \n"
        "vmrs r0, fpexc            \n"
        "orr r0, r0, #(1<<30)      \n"
        "vmsr fpexc, r0            \n"
        : : : "r0"
    );

//    init_platform();
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* 无定时器的纯事件驱动 ThreadX 启动！ */
//    tx_kernel_enter();
//    return 0;
    init_platform();
	*heartbeat_ptr = 0xDEADBEE2;

	/* ！！！在此激活底层硬件秒表 ！！！ */
	pmu_enable();

	/* 无定时器的纯事件驱动 ThreadX 启动！ */
	tx_kernel_enter();
	return 0;

}
