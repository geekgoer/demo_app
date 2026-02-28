/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation
 *
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 *
 * SPDX-License-Identifier: MIT
 **************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX Component                                                     */
/**                                                                       */
/**   User Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    tx_user.h                                         Cortex-R5/GNU     */
/*                                                           6.4.1        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring ThreadX in          */
/*    specific ways. This file will have an effect only if the            */
/*    application and ThreadX library are built with TX_INCLUDE_USER_-    */
/*    DEFINE_FILE defined. Note that all the defines in this file may     */
/*    also be made on the command line when building ThreadX library      */
/*    and application objects.                                            */
/*                                                                        */
/**************************************************************************/

#ifndef TX_USER_H
#define TX_USER_H


/* Define various build options for the ThreadX port.  The application should either make changes
   here by commenting or un-commenting the conditional compilation defined OR supply the defines
   though the compiler's equivalent of the -D option.  */


/* Override various options with default values already assigned in tx_port.h. */

/* Determine if timer expirations (application timers, timeouts, and tx_thread_sleep calls
   should be processed within the a system thread rather than the timer ISR. By default, the
   timer thread is disabled. When the following is defined, the timer thread will be created
   and used to process timer expirations. */

/* #define TX_TIMER_PROCESS_IN_ISR */

/* Determine if no timer processing is required. This option will help eliminate the timer
   processing when not needed. The user will have to provide their own timer interrupt
   at the desired rate and call the void _tx_timer_interrupt(void); routine. */

/* #define TX_NO_TIMER */

/* Define TX_DISABLE_ERROR_CHECKING if you want to disable ThreadX error checking. */

/* #define TX_DISABLE_ERROR_CHECKING */

/* Define the maximum number of thread priorities.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32.  */

/* #define TX_MAX_PRIORITIES                       32 */

/* Define the minimum stack size for threads on this processor. If the size supplied during
   thread creation is less than this value, the thread create call will return an error.  */

/* #define TX_MINIMUM_STACK                        200 */

/* Define the system timer thread's default stack size and priority.  These are only applicable
   if TX_TIMER_PROCESS_IN_ISR is not defined.  */

/* #define TX_TIMER_THREAD_STACK_SIZE              1024 */
/* #define TX_TIMER_THREAD_PRIORITY                0 */

/* Define various constants for the ThreadX Cortex-R5 port.  */

/* Define interrupt lockout and restore macros for protection on
   access of critical sections.  */

/* #define TX_DISABLE */
/* #define TX_RESTORE */

/* Define the interrupt lockout level. If not defined, ARM CPSR register
   settings are used to disable interrupts. */

/* #define TX_INT_DISABLE                          0x80 */
/* #define TX_INT_ENABLE                           0x00 */


/* Determine if block pool performance gathering is required by the application.  When the following is
   defined, ThreadX gathers various block pool performance information. */

/* #define TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO */


/* Determine if byte pool performance gathering is required by the application.  When the following is
   defined, ThreadX gathers various byte pool performance information. */

/* #define TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO */


/* Determine if event flags performance gathering is required by the application.  When the following is
   defined, ThreadX gathers various event flags performance information. */

/* #define TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO */


/* Determine if mutex performance gathering is required by the application.  When the following is
   defined, ThreadX gathers various mutex performance information. */

/* #define TX_MUTEX_ENABLE_PERFORMANCE_INFO */


/* Determine if queue performance gathering is required by the application.  When the following is
   defined, ThreadX gathers various queue performance information. */

/* #define TX_QUEUE_ENABLE_PERFORMANCE_INFO */


/* Determine if semaphore performance gathering is required by the application.  When the following is
   defined, ThreadX gathers various semaphore performance information. */

/* #define TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO */


/* Determine if thread performance gathering is required by the application.  When the following is
   defined, ThreadX gathers various thread performance information. */

/* #define TX_THREAD_ENABLE_PERFORMANCE_INFO */


/* Determine if timer performance gathering is required by the application.  When the following is
   defined, ThreadX gathers various timer performance information. */

/* #define TX_TIMER_ENABLE_PERFORMANCE_INFO */


/* Determine if the notify callback option for all ThreadX objects is required by the application.
   By default, this is commented out. */

/* #define TX_ENABLE_EVENT_TRACE */


/* Determine if stack checking is enabled. By default, ThreadX stack checking is
   disabled. When the following is defined, ThreadX thread stack checking is enabled.
   If stack checking is enabled (TX_ENABLE_STACK_CHECKING is defined), the TX_DISABLE_STACK_FILLING
   define is negated, thereby forcing the stack fill which is necessary for the stack checking logic. */

/* #define TX_ENABLE_STACK_CHECKING */


/* Determine if preemption-threshold should be disabled. By default, preemption-threshold is
   enabled. If the application does not use preemption-threshold, it may be disabled to reduce
   code size and improve performance. */

/* #define TX_DISABLE_PREEMPTION_THRESHOLD */


/* Determine if redundant clearing should be disabled. By default, ThreadX clears ULONG and
   pointer local variables to zero prior to use. This processing overhead may be eliminated
   by uncommenting the following line. */

/* #define TX_DISABLE_REDUNDANT_CLEARING */


/* Determine if inline is allowed in the ThreadX source code. By default, inline is disabled.
   When the following is defined, inline is enabled. */

/* #define TX_INLINE_THREAD_RESUME_SUSPEND */


/* Define the version ID of ThreadX. This may be utilized by the application. */

#ifdef TX_USER_H_VERSION_ID
#undef TX_USER_H_VERSION_ID
#endif
#define TX_USER_H_VERSION_ID    "6.4.1"


#endif
