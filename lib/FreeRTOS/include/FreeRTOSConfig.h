#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Имена обработчиков */
#define xPortSysTickHandler SysTick_Handler
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler

/*-----------------------------------------------------------
 * Application specific definitions for STM32F103C8
 *----------------------------------------------------------*/

/* Ensure stdint.h is included */
#include <stdint.h>

/*-----------------------------------------------------------
 * Core configuration
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ( 72000000UL )
#define configTICK_RATE_HZ                      ( 1000UL )
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 10 * 1024 ) )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS	1
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configUSE_TASK_NOTIFICATIONS            1

/*-----------------------------------------------------------
 * Interrupt priority configuration (Cortex-M3)
 *----------------------------------------------------------*/

#define configPRIO_BITS                         4

/* The lowest interrupt priority that can be used in a call to a "set priority" function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0x0f

/* The highest interrupt priority that can be used by any interrupt that calls a FreeRTOS API function. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* These are the actual values used by the kernel */
#define configKERNEL_INTERRUPT_PRIORITY         ( 255UL )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 191UL )

/*-----------------------------------------------------------
 * Run time statistics - DISABLED
 *----------------------------------------------------------*/

#define configGENERATE_RUN_TIME_STATS           0
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE()        0

/*-----------------------------------------------------------
 * Timer (Software Timer) configuration
 *----------------------------------------------------------*/

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( 2 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE )

/*-----------------------------------------------------------
 * Co-routine definitions
 *----------------------------------------------------------*/

#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/*-----------------------------------------------------------
 * Hook function related definitions
 *----------------------------------------------------------*/

#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_APPLICATION_TASK_TAG          0

/*-----------------------------------------------------------
 * Optional functions
 *----------------------------------------------------------*/

#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xTimerPendFunctionCall          0
#define INCLUDE_xTaskAbortDelay                 0

/*-----------------------------------------------------------
 * Memory allocation
 *----------------------------------------------------------*/

#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1

/*-----------------------------------------------------------
 * Assertion
 *----------------------------------------------------------*/
#define configASSERT( x )

#endif /* FREERTOS_CONFIG_H */