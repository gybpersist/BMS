#ifndef __APP_TASK_H__
#define __APP_TASK_H__

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Int_BQ769.h"
#include "App_display.h"
#include "Int_CAN.h"
#include "string.h"

/**
 * @brief 启动FreeRTOS
 */
void App_Task_Start(void);

/**
 * @brief 启动任务的任务函数
 * 
 * @param pvParameters 
 */

void start_task(void *pvParameters);

#endif /* __APP_TASK_H__ */
