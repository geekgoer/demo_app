/******************************************************************************
*
* ThreadX + AMP Demo Application for Zynq UltraScale+ MPSoC R5
*
* This application demonstrates ThreadX RTOS with RPMsg communication to Linux
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xscugic.h"
#include "xttcps.h"
#include "tx_api.h"
#include "xpseudo_asm.h"  /* For VBAR register access */

/* OpenAMP includes */
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rsc_table.h"

/* Define ThreadX constants */
#define DEMO_STACK_SIZE         2048
#define RPMSG_STACK_SIZE        4096
#define DEMO_BYTE_POOL_SIZE     32768
#define DEMO_PRIORITY           10
#define RPMSG_PRIORITY          5

/* Timer defines */
#define TTC_DEVICE_ID           XPAR_XTTCPS_0_DEVICE_ID
#define TTC_TICK_INTR_ID        XPAR_XTTCPS_0_INTR
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID

#define SHUTDOWN_MSG            0xEF56A55A
#define LPRINTF(format, ...)    xil_printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...)    LPRINTF("ERROR: " format, ##__VA_ARGS__)



/* Define the ThreadX object control blocks */
TX_THREAD               demo_thread;
TX_THREAD               rpmsg_thread;
TX_BYTE_POOL            byte_pool_0;
TX_MUTEX                rpmsg_mutex;

/* Define the counters used in the demo application */
ULONG                   demo_thread_counter;
ULONG                   rpmsg_msg_counter;

/* Diagnostic addresses in R5 reserved DDR tail - reliable from both sides.
 * OCM (0xFFFC0000) is used by PMU firmware, unreliable for diagnostics. */
#define HEARTBEAT_ADDR  0x3EFF0000
#define DIAG_ADDR       0x3EFF0004
volatile uint32_t *heartbeat_ptr = (volatile uint32_t *)HEARTBEAT_ADDR;
volatile uint32_t *diag_ptr      = (volatile uint32_t *)DIAG_ADDR;

/* Xilinx hardware instances */
static XScuGic          InterruptController;
static XTtcPs           TimerInstance;

/* OpenAMP variables */
static struct rpmsg_endpoint lept;
static int shutdown_req = 0;
static struct rpmsg_device *rpdev_global = NULL;
static void *platform_global = NULL;

/* Define thread prototypes */
void    demo_thread_entry(ULONG thread_input);
void    rpmsg_thread_entry(ULONG thread_input);
void    setup_timer_interrupt(void);
void    timer_interrupt_handler(void *CallBackRef);

/* ThreadX timer interrupt - external declaration */
extern void _tx_timer_interrupt(void);

/* OpenAMP function prototypes */
extern int init_system(void);
extern void cleanup_system(void);
int rpmsg_app_init(void);
void rpmsg_app_cleanup(void);

/* Define memory pool */
UCHAR   memory_area[DEMO_BYTE_POOL_SIZE];

/**************************************************************************/
/*  RPMsg endpoint callback                                               */
/**************************************************************************/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                             uint32_t src, void *priv)
{
    (void)priv;
    (void)src;

    /* Check for shutdown message */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
        LPRINTF("Shutdown message received.\r\n");
        shutdown_req = 1;
        return RPMSG_SUCCESS;
    }

    /* Build response with ThreadX proof: tick count + thread name + msg counter */
    char response[256];
    ULONG tx_ticks = tx_time_get();
    TX_THREAD *current_thread = tx_thread_identify();
    const char *thread_name = current_thread ? current_thread->tx_thread_name : "none";

    int resp_len = snprintf(response, sizeof(response),
                            "TX_TICK=%lu THREAD=%s MSG#%lu DATA=%.*s",
                            tx_ticks, thread_name,
                            rpmsg_msg_counter + 1,
                            (int)len, (char *)data);

    if (rpmsg_send(ept, response, resp_len) < 0) {
        LPERROR("rpmsg_send failed\r\n");
    } else {
        rpmsg_msg_counter++;
    }

    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    LPRINTF("Unexpected remote endpoint destroy\r\n");
    shutdown_req = 1;
}

/**************************************************************************/
/*  Initialize RPMsg application                                          */
/**************************************************************************/
int rpmsg_app_init(void)
{
    int ret;

    *diag_ptr = 0xA0000001;  /* reached rpmsg_app_init */

    /* Initialize platform */
    ret = platform_init(0, NULL, &platform_global);
    if (ret) {
        *diag_ptr = 0xE0000001;  /* platform_init FAILED */
        return -1;
    }
    *diag_ptr = 0xA0000002;  /* platform_init OK */

    rpdev_global = platform_create_rpmsg_vdev(platform_global, 0,
                                               VIRTIO_DEV_SLAVE,
                                               NULL, NULL);
    if (!rpdev_global) {
        *diag_ptr = 0xE0000002;  /* create_rpmsg_vdev FAILED */
        return -1;
    }
    *diag_ptr = 0xA0000003;  /* create_rpmsg_vdev OK */

    /* Create RPMsg endpoint */
    ret = rpmsg_create_ept(&lept, rpdev_global, "rpmsg-openamp-demo-channel",
                           0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb,
                           rpmsg_service_unbind);
    if (ret) {
        *diag_ptr = 0xE0000003;  /* rpmsg_create_ept FAILED */
        return -1;
    }
    *diag_ptr = 0xA0000004;  /* rpmsg_create_ept OK - endpoint created! */

    LPRINTF("RPMsg endpoint created successfully.\r\n");
    LPRINTF("Waiting for messages from Linux...\r\n");

    return 0;
}

/**************************************************************************/
/*  Cleanup RPMsg application                                             */
/**************************************************************************/
void rpmsg_app_cleanup(void)
{
    if (rpdev_global) {
        rpmsg_destroy_ept(&lept);
        platform_release_rpmsg_vdev(rpdev_global);
    }

    if (platform_global) {
        platform_cleanup(platform_global);
    }
}

/**************************************************************************/
/*  Demo thread entry                                                     */
/**************************************************************************/
void demo_thread_entry(ULONG thread_input)
{
    (void)thread_input;

    LPRINTF("Demo thread started\r\n");

    while (1) {
        /* Increment counter */
        demo_thread_counter++;

        /* Update heartbeat to show ThreadX is running */
        *heartbeat_ptr = 0x12345678 + demo_thread_counter;

        /* Print status every 10 seconds */
        if (demo_thread_counter % 100 == 0) {
            LPRINTF("Demo thread: counter = %lu, rpmsg messages = %lu\r\n",
                    demo_thread_counter, rpmsg_msg_counter);
        }

        /* Sleep for 100ms */
        tx_thread_sleep(10);
    }
}

/**************************************************************************/
/*  RPMsg thread entry                                                    */
/**************************************************************************/
void rpmsg_thread_entry(ULONG thread_input)
{
    (void)thread_input;

    LPRINTF("RPMsg thread started\r\n");

    /* Initialize RPMsg */
    if (rpmsg_app_init() != 0) {
        LPERROR("RPMsg initialization failed!\r\n");
        return;
    }

    /* Main RPMsg loop */
    while (!shutdown_req) {
        platform_poll(platform_global);
        tx_thread_sleep(1);  /* Yield to other threads */
    }

    LPRINTF("RPMsg thread shutting down...\r\n");
    rpmsg_app_cleanup();
}

/**************************************************************************/
/*  Timer interrupt handler                                               */
/**************************************************************************/
void timer_interrupt_handler(void *CallBackRef)
{
    XTtcPs *TtcPsInst = (XTtcPs *)CallBackRef;
    u32 StatusEvent;

    /* Read and clear the interrupt status */
    StatusEvent = XTtcPs_GetInterruptStatus(TtcPsInst);
    XTtcPs_ClearInterruptStatus(TtcPsInst, StatusEvent);

    /* Call ThreadX timer interrupt */
    _tx_timer_interrupt();
}

/**************************************************************************/
/*  Setup timer interrupt                                                 */
/**************************************************************************/
void setup_timer_interrupt(void)
{
    int Status;
    XTtcPs_Config *Config;
    XScuGic_Config *IntcConfig;
    XInterval Interval;  /* Changed from u16 to XInterval (u32) */
    u8 Prescaler;

    /* Initialize the interrupt controller */
    IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
    Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
                                   IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("Interrupt controller init failed\r\n");
        return;
    }

    /* Initialize the timer */
    Config = XTtcPs_LookupConfig(TTC_DEVICE_ID);
    Status = XTtcPs_CfgInitialize(&TimerInstance, Config, Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("Timer init failed\r\n");
        return;
    }

    /* Set timer options */
    XTtcPs_SetOptions(&TimerInstance, XTTCPS_OPTION_INTERVAL_MODE |
                                      XTTCPS_OPTION_WAVE_DISABLE);

    /* Calculate timer interval for 10ms (100Hz) */
    XTtcPs_CalcIntervalFromFreq(&TimerInstance, 100, &Interval, &Prescaler);
    XTtcPs_SetInterval(&TimerInstance, Interval);
    XTtcPs_SetPrescaler(&TimerInstance, Prescaler);

    /* Connect timer interrupt */
    Status = XScuGic_Connect(&InterruptController, TTC_TICK_INTR_ID,
                            (Xil_ExceptionHandler)timer_interrupt_handler,
                            (void *)&TimerInstance);
    if (Status != XST_SUCCESS) {
        xil_printf("Timer interrupt connect failed\r\n");
        return;
    }

    /* Enable timer interrupt */
    XScuGic_Enable(&InterruptController, TTC_TICK_INTR_ID);
    XTtcPs_EnableInterrupts(&TimerInstance, XTTCPS_IXR_INTERVAL_MASK);

    /* Register interrupt controller handler */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 &InterruptController);

    /* Enable interrupts */
    Xil_ExceptionEnable();

    /* Start the timer */
    XTtcPs_Start(&TimerInstance);
}

/**************************************************************************/
/*  Application define function                                           */
/**************************************************************************/
void tx_application_define(void *first_unused_memory)
{
    CHAR *pointer;
    UINT status;

    /* Create a byte memory pool */
    status = tx_byte_pool_create(&byte_pool_0, "byte pool 0", memory_area,
                                 DEMO_BYTE_POOL_SIZE);
    if (status != TX_SUCCESS) {
        xil_printf("Failed to create byte pool\r\n");
        return;
    }

    /* Create mutex for RPMsg */
    status = tx_mutex_create(&rpmsg_mutex, "rpmsg mutex", TX_NO_INHERIT);
    if (status != TX_SUCCESS) {
        xil_printf("Failed to create mutex\r\n");
        return;
    }

    /* Allocate stack for demo thread */
    status = tx_byte_allocate(&byte_pool_0, (VOID **)&pointer,
                              DEMO_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        xil_printf("Failed to allocate demo thread stack\r\n");
        return;
    }

    /* Create demo thread */
    status = tx_thread_create(&demo_thread, "Demo Thread", demo_thread_entry, 0,
                              pointer, DEMO_STACK_SIZE,
                              DEMO_PRIORITY, DEMO_PRIORITY,
                              TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        xil_printf("Failed to create demo thread\r\n");
        return;
    }

    /* Allocate stack for RPMsg thread */
    status = tx_byte_allocate(&byte_pool_0, (VOID **)&pointer,
                              RPMSG_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        xil_printf("Failed to allocate rpmsg thread stack\r\n");
        return;
    }

    /* Create RPMsg thread */
    status = tx_thread_create(&rpmsg_thread, "RPMsg Thread", rpmsg_thread_entry, 0,
                              pointer, RPMSG_STACK_SIZE,
                              RPMSG_PRIORITY, RPMSG_PRIORITY,
                              TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
        xil_printf("Failed to create rpmsg thread\r\n");
        return;
    }

    xil_printf("ThreadX application defined successfully\r\n");
}

/**************************************************************************/
/*  Main entry point                                                      */
/**************************************************************************/
//int main()
//{
//    /* Step 1: Write heartbeat FIRST - proves R5 reached main() */
//    *heartbeat_ptr = 0xDEADBEEF;
//
//    /* Step 2: Set VBAR to point to DDR vector table */
//    extern u32 _vector_table;
//    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"((u32)&_vector_table));
//    __asm__ volatile("dsb");
//    __asm__ volatile("isb");
//
//    /* Step 3: Disable caches for AMP shared memory coherency */
//    Xil_DCacheDisable();
//    Xil_ICacheDisable();
//
//    *heartbeat_ptr = 0xDEADBEE1;
//
//    /* Step 4: Initialize platform (safe no-op for R5 cache functions) */
//    init_platform();
//
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* Step 5: Enter ThreadX kernel - creates threads via tx_application_define() */
//    tx_kernel_enter();
//
//    /* Should never reach here */
//    return 0;
//}
/**************************************************************************/
/* Main entry point                                                      */
/**************************************************************************/
//int main()
//{
//    /* 证锟斤拷 R5 锟缴癸拷锟斤拷锟斤拷锟斤拷 main() */
//    *heartbeat_ptr = 0xDEADBEE1;
//
//    /* 锟斤拷始锟斤拷平台硬锟斤拷锟斤拷锟界串锟节等ｏ拷锟阶诧拷锟窖讹拷 R5 锟斤拷锟剿帮拷全锟斤拷锟诫） */
//    init_platform();
//
//    /* 证锟斤拷平台锟斤拷始锟斤拷锟斤拷希锟阶硷拷锟斤拷锟斤拷锟� ThreadX */
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* 锟斤拷锟斤拷 ThreadX 锟斤拷锟斤拷锟斤拷 - 锟斤拷锟斤拷锟斤拷锟� tx_application_define 锟斤拷锟斤拷锟斤拷锟竭筹拷 */
//    tx_kernel_enter();
//
//    /* ThreadX 锟斤拷锟斤拷锟斤拷庸锟� CPU锟斤拷锟斤拷远锟斤拷锟斤拷执锟叫碉拷锟斤拷锟斤拷 */
//    return 0;
//}
//int main()
//{
//    /* 1. 证锟斤拷 R5 锟缴癸拷锟斤拷锟斤拷锟斤拷 main() */
//    *heartbeat_ptr = 0xDEADBEE1;
//
//    /* 2. 锟斤拷锟斤拷锟斤拷药锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 VBAR锟斤拷锟斤拷锟斤拷一锟斤拷锟叫讹拷 CPU 锟斤拷锟斤拷锟斤拷 TCM 锟杰飞ｏ拷 */
//    extern u32 _vector_table;
//    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"((u32)&_vector_table));
//    __asm__ volatile("dsb");
//    __asm__ volatile("isb");
//
//    /* 锟斤拷锟皆诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟� Cache (Xil_DCacheDisable)锟斤拷锟斤拷锟斤拷锟斤拷锟� Data Abort锟斤拷 */
//
//    /* 3. 锟斤拷始锟斤拷平台硬锟斤拷 */
//    init_platform();
//
//    /* 4. 证锟斤拷平台锟斤拷始锟斤拷锟斤拷希锟阶硷拷锟斤拷锟斤拷锟� ThreadX */
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* 5. 锟斤拷锟斤拷 ThreadX 锟斤拷锟斤拷锟斤拷 */
//    tx_kernel_enter();
//
//    return 0;
//}
//int main()
//{
//    /* 1. 证锟斤拷锟斤拷锟斤拷 main */
//    *heartbeat_ptr = 0xDEADBEE1;
//
//    /* 2. 锟斤拷锟斤拷 CPU 锟叫讹拷锟斤拷锟斤拷锟斤拷锟斤拷 DDR (锟斤拷锟斤拷锟斤拷要) */
//    extern u32 _vector_table;
//    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"((u32)&_vector_table));
//    __asm__ volatile("dsb");
//    __asm__ volatile("isb");
//
//    /* 删锟斤拷锟斤拷 init_platform(); 锟斤拷锟皆诧拷去锟斤拷 Linux 锟斤拷硬锟斤拷锟斤拷 */
//
//    /* 3. 证锟斤拷锟斤拷锟斤拷锟竭碉拷锟斤拷 ThreadX 锟斤拷前 */
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* 4. 锟斤拷锟斤拷 ThreadX 锟斤拷锟斤拷锟斤拷 */
//    tx_kernel_enter();
//
//    return 0;
//}
//int main()
//{
//    /* 1. 写锟斤拷探锟诫，锟斤拷强锟斤拷刷锟斤拷锟斤拷锟斤拷锟节存（锟斤拷散锟斤拷锟介） */
//    *heartbeat_ptr = 0xDEADBEE1;
//    Xil_DCacheFlushRange((UINTPTR)heartbeat_ptr, 4);
//
//    /* 2. 锟斤拷锟斤拷 CPU 锟叫讹拷锟斤拷锟斤拷锟斤拷锟斤拷 DDR (锟斤拷锟斤拷锟斤拷药) */
//    extern u32 _vector_table;
//    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"((u32)&_vector_table));
//    __asm__ volatile("dsb");
//    __asm__ volatile("isb");
//
//    /* 3. 锟街革拷 init_platform锟斤拷锟斤拷 R5 锟斤拷锟铰筹拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷汀锟� */
//    init_platform();
//
//    /* 4. 证锟斤拷锟缴癸拷挺锟斤拷锟斤拷平台锟斤拷始锟斤拷 */
//    *heartbeat_ptr = 0xDEADBEE2;
//    Xil_DCacheFlushRange((UINTPTR)heartbeat_ptr, 4);
//
//    /* 5. 锟斤拷锟斤拷 ThreadX 锟斤拷锟斤拷锟斤拷 */
//    tx_kernel_enter();
//
//    return 0;
//}
//int main()
//{
//    /* 1. 写锟斤拷探锟诫并刷锟斤拷锟斤拷锟斤拷锟节达拷 */
//    *heartbeat_ptr = 0xDEADBEE1;
//    Xil_DCacheFlushRange((UINTPTR)heartbeat_ptr, 4);
//
//    /* 2. 锟斤拷锟斤拷锟叫讹拷锟斤拷锟斤拷锟斤拷 */
//    extern u32 _vector_table;
//    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"((u32)&_vector_table));
//    __asm__ volatile("dsb");
//    __asm__ volatile("isb");
//
//    /* 锟斤拷锟皆诧拷要写 init_platform()锟斤拷 */
//
//    /* 3. 证锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟� */
//    *heartbeat_ptr = 0xDEADBEE2;
//    Xil_DCacheFlushRange((UINTPTR)heartbeat_ptr, 4);
//
//    /* 4. 锟斤拷锟斤拷 ThreadX */
//    tx_kernel_enter();
//
//    return 0;
//}
//int main()
//{
//    /* 1. 探锟诫：证锟斤拷锟斤拷锟斤拷 main (锟斤拷选锟斤拷锟斤拷锟斤拷锟睫凤拷) */
//    *heartbeat_ptr = 0xDEADBEE1;
//
//    /* 2. 锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷锟叫讹拷锟斤拷锟斤拷锟斤拷指锟斤拷 DDR锟斤拷锟斤拷锟斤拷一锟斤拷锟叫断撅拷锟杰飞碉拷 TCM */
//    extern u32 _vector_table;
//    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"((u32)&_vector_table));
//    __asm__ volatile("dsb");
//    __asm__ volatile("isb");
//
//    /* 3. 锟斤拷锟斤拷锟斤拷始锟斤拷硬锟斤拷平台锟斤拷锟斤拷锟斤拷 Cache锟斤拷锟斤拷始锟斤拷锟斤拷锟节ｏ拷 */
//    init_platform();
//
//    /* 4. 证锟斤拷锟斤拷锟斤拷锟剿筹拷始锟斤拷 */
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* 5. 锟斤拷锟斤拷 ThreadX 锟斤拷锟斤拷锟斤拷 */
//    tx_kernel_enter();
//
//    return 0;
//}
//int main()
//{
//    /* Step 1: Write heartbeat to prove R5 reached main() */
//    *heartbeat_ptr = 0xDEADBEE1;
//
//    /* Step 2: Set VBAR to point to DDR vector table */
//    extern u32 _vector_table;
//    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"((u32)&_vector_table));
//    __asm__ volatile("dsb");
//    __asm__ volatile("isb");
//
//    /* Step 3: Initialize platform (GIC, IPI for OpenAMP) */
//    init_platform();
//
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* Step 4: Setup timer interrupt for ThreadX */
//    setup_timer_interrupt();
//
//    /* Step 5: Enter ThreadX kernel */
//    tx_kernel_enter();
//
//    /* Should never reach here */
//    return 0;
//}
/**************************************************************************/
/* Main entry point                                                      */
/**************************************************************************/
//int main()
//{
//    /* 证明 R5 成功进入了 main() */
//    *heartbeat_ptr = 0xDEADBEE1;
//
//    /* 初始化平台硬件（如串口等，底层已对 R5 做了安全隔离） */
//    init_platform();
//
//    /* 证明平台初始化完毕，准备进入 ThreadX */
//    *heartbeat_ptr = 0xDEADBEE2;
//
//    /* 启动 ThreadX 调度器 - 将会调用 tx_application_define 来创建线程 */
//    tx_kernel_enter();
//
//    /* ThreadX 启动后接管 CPU，永远不会执行到这里 */
//    return 0;
//}
int main()
{
    /* 1. 探针证明进入 main，并立刻刷入物理内存 */
    *heartbeat_ptr = 0xDEADBEE1;
    Xil_DCacheFlushRange((UINTPTR)heartbeat_ptr, 4);

    /* 2. 手工复制中断向量表到 TCM (0x0)，这一步已经被你证明完美生效！ */
    extern u32 _vector_table;
    volatile u32 *src = (volatile u32 *)&_vector_table;
    volatile u32 *dst = (volatile u32 *)0x0;
    for (int i = 0; i < 16; i++) {
        dst[i] = src[i];
    }
    __asm__ volatile("dsb");
    __asm__ volatile("isb");

    /* 3. 【终极解药】：手动开启 R5 的硬件 FPU (协处理器 CP10 和 CP11)
     * 没有这一步，ThreadX 一调度就会触发 0xdb 未定义指令异常死机！ */
    __asm__ volatile(
        "mrc p15, 0, r0, c1, c0, 2 \n" // 读取 CPACR 寄存器
        "orr r0, r0, #(0xF << 20)  \n" // 授予 CP10 和 CP11 满权限
        "mcr p15, 0, r0, c1, c0, 2 \n" // 写回 CPACR
        "isb                       \n"
        "vmrs r0, fpexc            \n" // 读取 FPEXC 寄存器
        "orr r0, r0, #(1<<30)      \n" // 设置 EN (开启) 位
        "vmsr fpexc, r0            \n" // 写回 FPEXC
        : : : "r0"
    );

    /* 4. 恢复定时器中断 (ThreadX 需要它来推动线程调度和休眠) */
    setup_timer_interrupt();

    /* 5. 证明我们活着走出了所有的硬件坑！ */
    *heartbeat_ptr = 0xDEADBEE2;
    Xil_DCacheFlushRange((UINTPTR)heartbeat_ptr, 4);

    /* 6. 启动 ThreadX 调度器 */
    tx_kernel_enter();

    return 0;
}
