#include "App_Task.h"

// 0. 启动任务的配置
#define START_TASK_NAME "start_task"
#define START_TASK_STACK 128
#define START_TASK_PRIORITY 10
TaskHandle_t start_task_handle;
void start_task(void *pvParameters); // 启动任务函数

// 1. BQ芯片的任务配置
#define BQ_TASK_NAME "bq_task"
#define BQ_TASK_STACK 128
#define BQ_TASK_PRIORITY 2
TaskHandle_t bq_task_handle;
void bq_task(void *pvParameters); // 任务函数

// 2. 显示屏芯片的任务配置
#define DISPLAY_TASK_NAME "display_task"
#define DISPLAY_TASK_STACK 128
#define DISPLAY_TASK_PRIORITY 1
TaskHandle_t display_task_handle;
void display_task(void *pvParameters); // 任务函数

// 3. CAN通信的任务配置
#define CAN_TASK_NAME "can_task"
#define CAN_TASK_STACK 128
#define CAN_TASK_PRIORITY 3
TaskHandle_t can_task_handle;
void can_task(void *pvParameters); // 任务函数

/* 其他任务的配置 */
/* code */

/**
 * @brief 启动FreeRTOS
 */
void App_Task_Start(void)
{
    // 0. 输出提示
    DEBUG_PRINTLN("BMS电池管理项目....");

    // 1. 创建一个启动任务,这个任务负责创建其他任务
    BaseType_t result = xTaskCreate(
        (TaskFunction_t)start_task,
        (char *)"start_task",
        (configSTACK_DEPTH_TYPE)START_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)START_TASK_PRIORITY,
        (TaskHandle_t *)&start_task_handle);

    if (result == pdPASS)
    {
        DEBUG_PRINTLN("启动任务创建成功...");
    }
    else
    {
        DEBUG_PRINTLN("启动任务创建失败...");
    }

    // 2. 启动调度器(内部会自动创建空闲任务)
    vTaskStartScheduler();
}

// 启动任务的任务函数
void start_task(void *pvParameters)
{
    DEBUG_PRINTLN("启动任务开始执行:开始创建其他任务....");
    // 1.进入临界区
    taskENTER_CRITICAL();

    // 2. 创建BQ芯片的任务
    BaseType_t result = xTaskCreate(
        (TaskFunction_t)bq_task,
        (char *)"bq_task",
        (configSTACK_DEPTH_TYPE)BQ_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)BQ_TASK_PRIORITY,
        (TaskHandle_t *)&bq_task_handle);
    if (result == pdPASS)
    {
        DEBUG_PRINTLN("BQ芯片任务创建成功...");
    }
    else
    {
        DEBUG_PRINTLN("BQ芯片任务创建失败...");
    }

    // 3. 创建显示屏的任务
    result = xTaskCreate(
        (TaskFunction_t)display_task,
        (char *)"display_task",
        (configSTACK_DEPTH_TYPE)DISPLAY_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)DISPLAY_TASK_PRIORITY,
        (TaskHandle_t *)&display_task_handle);
    if (result == pdPASS)
    {
        DEBUG_PRINTLN("显示屏任务创建成功...");
    }
    else
    {
        DEBUG_PRINTLN("显示屏任务创建失败...");
    }

    // 4. CAN通信的任务
    result = xTaskCreate(
        (TaskFunction_t)can_task,
        (char *)"can_task",
        (configSTACK_DEPTH_TYPE)CAN_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)CAN_TASK_PRIORITY,
        (TaskHandle_t *)&can_task_handle);
    if (result == pdPASS)
    {
        DEBUG_PRINTLN("CAN通信任务创建成功...");
    }
    else
    {
        DEBUG_PRINTLN("CAN通信任务创建失败...");
    }

    // 退出临界区
    taskEXIT_CRITICAL();

    // 4. 删除启动任务
    vTaskDelete(NULL);
}

// 保存读取到的电池电压
float read_CellVoltage_buff[CELL_NUM] = {0};
// 保存读取到的电流值
float read_current = 0;
// 保存读取到的电池盒的电压值
float read_PackVoltage = 0;
// 保存读取到的温度
float temperature = 0;
// 保存读取到的剩余电量
float read_SOC = 0;

void bq_task(void *pvParameters)
{
    // 初始化 BQ769芯片
    Int_BQ769_Init();

    while (1)
    {
        for (uint8_t i = 0; i < CELL_NUM; i++)
        {
            Int_BQ769_GetCellVoltage(i, &read_CellVoltage_buff[i]);
        }
        debug_printfln("V: %.02f %.02f %.02f %.02f %.02f",
                       read_CellVoltage_buff[0],
                       read_CellVoltage_buff[1],
                       read_CellVoltage_buff[2],
                       read_CellVoltage_buff[3],
                       read_CellVoltage_buff[4]);

        Int_BQ769_GetCurrent(&read_current);
        debug_printfln("I:%.02f", read_current);

        Int_BQ769_GetPackVoltage(&read_PackVoltage);
        debug_printfln("Pack V:%.02f", read_PackVoltage);

        Int_BQ769_GetTemperature(&temperature);
        debug_printfln("temperature:%.02f", temperature);

        Int_BQ769_GetSOC(read_CellVoltage_buff, &read_SOC, temperature);
        debug_printfln("SOC: %.02f", read_SOC);

        vTaskDelay(500);
    }
}

void display_task(void *pvParameters)
{
    // 显示屏初始化
    App_display_Init();

    while (1)
    {
        App_display_handle();
        vTaskDelay(1000);
    }
}

void can_task(void *pvParameters)
{
    Int_CAN_Init();
    while (1)
    {
        RxDataMsg_Info RxDataMsg[8];
        uint8_t msg_cnt = 0;
        Int_CAN_Receive_Msg(RxDataMsg, &msg_cnt);
        if (msg_cnt > 0)
        {
            for (uint8_t i = 0; i < msg_cnt; i++)
            {
                debug_printfln("I: %d,id: %d,data: %s,len: %d\r\n", i, RxDataMsg[i].id, RxDataMsg[i].data, RxDataMsg[i].len);

                if (strcmp((char *)RxDataMsg[i].data, "D:1") == 0)
                {
                    // 开启放电控制
                    Int_BQ769_DisChargeCtrl(1);
                    debug_printfln("can:%s",RxDataMsg[i].data);
                }
                if (strcmp((char *)RxDataMsg[i].data, "D:0") == 0)
                {
                    // 不开启放电控制
                    Int_BQ769_DisChargeCtrl(0);
                    debug_printfln("can:%s",RxDataMsg[i].data);
                }
                if (strcmp((char *)RxDataMsg[i].data, "C:1") == 0)
                {
                    // 开启充电控制
                    Int_BQ769_ChargeCtrl(1);
                    debug_printfln("can:%s",RxDataMsg[i].data);
                }
                if (strcmp((char *)RxDataMsg[i].data, "C:0") == 0)
                {
                    // 关闭充电控制
                    Int_BQ769_ChargeCtrl(0);
                    debug_printfln("can:%s",RxDataMsg[i].data);
                }
            }
        }

        vTaskDelay(1000);
    }
}
