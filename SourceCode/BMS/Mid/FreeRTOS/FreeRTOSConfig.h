/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

// 开启抢占式调度器
#define configUSE_PREEMPTION 1
// 使能空闲任务钩子函数, 无默认需定义
#define configUSE_IDLE_HOOK 0
// 使能系统时钟节拍中断钩子函数, 无默认需定义
#define configUSE_TICK_HOOK 0
// 定义CPU主频, 单位: Hz, 无默认需定义
#define configCPU_CLOCK_HZ ((unsigned long)72000000)
// 定义系统时钟节拍频率, 单位: Hz, 无默认需定义
#define configTICK_RATE_HZ ((TickType_t)1000)
// 定义最大优先级数, 最大优先级=configMAX_PRIORITIES-1, 无默认需定义
#define configMAX_PRIORITIES (32)
// 定义空闲任务的栈空间大小, 单位: Word, 无默认需定义
#define configMINIMAL_STACK_SIZE ((unsigned short)128)
// FreeRTOS堆中可用的RAM总量, 单位: Byte, 无默认需定义
#define configTOTAL_HEAP_SIZE ((size_t)(12 * 1024))
// 定义任务名最大字符数, 默认: 16
#define configMAX_TASK_NAME_LEN (32)
// 使能可视化跟踪调试, 默认: 0
#define configUSE_TRACE_FACILITY 0
// 定义系统时钟节拍计数器的数据类型为16位无符号数, 无默认需定义
#define configUSE_16_BIT_TICKS 0
// 使能在抢占式调度下,同优先级的任务能抢占空闲任务, 默认: 1
#define configIDLE_SHOULD_YIELD 1

// 设置任务优先级
#define INCLUDE_vTaskPrioritySet 1
// 获取任务优先级
#define INCLUDE_uxTaskPriorityGet 1
// 删除任务
#define INCLUDE_vTaskDelete 1
// 挂起任务
#define INCLUDE_vTaskSuspend 1
// 任务绝对延时
#define INCLUDE_vTaskDelayUntil 1
// 任务延时
#define INCLUDE_vTaskDelay 1

/* 设置 RTOS 内核自身使用的中断优先级。 一般设置为最低优先级, 不至于屏蔽其他优先级程序*/
// 该配置项在配置文件中已存在，修改原值，不要重复定义
#define configKERNEL_INTERRUPT_PRIORITY (15 << 4)
/* 设置了 调用中断安全的 FreeRTOS API 函数的最高中断优先级。 FreeRTOS 的管理的最高优先级 */
// 该配置项在配置文件中已存在，修改原值，不要重复定义
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (5 << 4)
/* 同上. 仅用于新版移植。 这两者是等效的。 */
#define configMAX_API_CALL_INTERRUPT_PRIORITY configMAX_SYSCALL_INTERRUPT_PRIORITY

// RTOS 对象可通过 RAM （从 FreeRTOS 堆中自动分配）创建
// 设置为 0，则 RTOS 对象只能通过应用程序编写者提供的 RAM 创建
// 默认为 1。
#define configSUPPORT_DYNAMIC_ALLOCATION 1

// 添加必须的三个宏
#define xPortPendSVHandler PendSV_Handler
#define vPortSVCHandler SVC_Handler
#define INCLUDE_xTaskGetSchedulerState 1

#endif /* FREERTOS_CONFIG_H */
