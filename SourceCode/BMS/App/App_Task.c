#include "App_Task.h"

// 0. �������������
#define START_TASK_NAME "start_task"
#define START_TASK_STACK 128
#define START_TASK_PRIORITY 10
TaskHandle_t start_task_handle;
void start_task(void *pvParameters); // ����������

// 1. BQоƬ����������
#define BQ_TASK_NAME "bq_task"
#define BQ_TASK_STACK 128
#define BQ_TASK_PRIORITY 2
TaskHandle_t bq_task_handle;
void bq_task(void *pvParameters); // ������

// 2. ��ʾ��оƬ����������
#define DISPLAY_TASK_NAME "display_task"
#define DISPLAY_TASK_STACK 128
#define DISPLAY_TASK_PRIORITY 1
TaskHandle_t display_task_handle;
void display_task(void *pvParameters); // ������

// 3. CANͨ�ŵ���������
#define CAN_TASK_NAME "can_task"
#define CAN_TASK_STACK 128
#define CAN_TASK_PRIORITY 3
TaskHandle_t can_task_handle;
void can_task(void *pvParameters); // ������

/* ������������� */
/* code */

/**
 * @brief ����FreeRTOS
 */
void App_Task_Start(void)
{
    // 0. �����ʾ
    DEBUG_PRINTLN("BMS��ع�����Ŀ....");

    // 1. ����һ����������,��������𴴽���������
    BaseType_t result = xTaskCreate(
        (TaskFunction_t)start_task,
        (char *)"start_task",
        (configSTACK_DEPTH_TYPE)START_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)START_TASK_PRIORITY,
        (TaskHandle_t *)&start_task_handle);

    if (result == pdPASS)
    {
        DEBUG_PRINTLN("�������񴴽��ɹ�...");
    }
    else
    {
        DEBUG_PRINTLN("�������񴴽�ʧ��...");
    }

    // 2. ����������(�ڲ����Զ�������������)
    vTaskStartScheduler();
}

// ���������������
void start_task(void *pvParameters)
{
    DEBUG_PRINTLN("��������ʼִ��:��ʼ������������....");
    // 1.�����ٽ���
    taskENTER_CRITICAL();

    // 2. ����BQоƬ������
    BaseType_t result = xTaskCreate(
        (TaskFunction_t)bq_task,
        (char *)"bq_task",
        (configSTACK_DEPTH_TYPE)BQ_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)BQ_TASK_PRIORITY,
        (TaskHandle_t *)&bq_task_handle);
    if (result == pdPASS)
    {
        DEBUG_PRINTLN("BQоƬ���񴴽��ɹ�...");
    }
    else
    {
        DEBUG_PRINTLN("BQоƬ���񴴽�ʧ��...");
    }

    // 3. ������ʾ��������
    result = xTaskCreate(
        (TaskFunction_t)display_task,
        (char *)"display_task",
        (configSTACK_DEPTH_TYPE)DISPLAY_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)DISPLAY_TASK_PRIORITY,
        (TaskHandle_t *)&display_task_handle);
    if (result == pdPASS)
    {
        DEBUG_PRINTLN("��ʾ�����񴴽��ɹ�...");
    }
    else
    {
        DEBUG_PRINTLN("��ʾ�����񴴽�ʧ��...");
    }

    // 4. CANͨ�ŵ�����
    result = xTaskCreate(
        (TaskFunction_t)can_task,
        (char *)"can_task",
        (configSTACK_DEPTH_TYPE)CAN_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)CAN_TASK_PRIORITY,
        (TaskHandle_t *)&can_task_handle);
    if (result == pdPASS)
    {
        DEBUG_PRINTLN("CANͨ�����񴴽��ɹ�...");
    }
    else
    {
        DEBUG_PRINTLN("CANͨ�����񴴽�ʧ��...");
    }

    // �˳��ٽ���
    taskEXIT_CRITICAL();

    // 4. ɾ����������
    vTaskDelete(NULL);
}

// �����ȡ���ĵ�ص�ѹ
float read_CellVoltage_buff[CELL_NUM] = {0};
// �����ȡ���ĵ���ֵ
float read_current = 0;
// �����ȡ���ĵ�غеĵ�ѹֵ
float read_PackVoltage = 0;
// �����ȡ�����¶�
float temperature = 0;
// �����ȡ����ʣ�����
float read_SOC = 0;

void bq_task(void *pvParameters)
{
    // ��ʼ�� BQ769оƬ
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
    // ��ʾ����ʼ��
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
                    // �����ŵ����
                    Int_BQ769_DisChargeCtrl(1);
                    debug_printfln("can:%s",RxDataMsg[i].data);
                }
                if (strcmp((char *)RxDataMsg[i].data, "D:0") == 0)
                {
                    // �������ŵ����
                    Int_BQ769_DisChargeCtrl(0);
                    debug_printfln("can:%s",RxDataMsg[i].data);
                }
                if (strcmp((char *)RxDataMsg[i].data, "C:1") == 0)
                {
                    // ����������
                    Int_BQ769_ChargeCtrl(1);
                    debug_printfln("can:%s",RxDataMsg[i].data);
                }
                if (strcmp((char *)RxDataMsg[i].data, "C:0") == 0)
                {
                    // �رճ�����
                    Int_BQ769_ChargeCtrl(0);
                    debug_printfln("can:%s",RxDataMsg[i].data);
                }
            }
        }

        vTaskDelay(1000);
    }
}
