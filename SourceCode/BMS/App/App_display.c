#include "App_display.h"

uint8_t display_buff[8];

/**
 * @brief ��ʾ����ʼ��
 *
 */
void App_display_Init(void)
{
    Inf_OLED_Init();
    Inf_OLED_ShowString(32, 0, "BMS", 16, 1);
    Inf_OLED_ShowChinese(56, 0, 5, 16, 1);
    Inf_OLED_ShowChinese(72, 0, 6, 16, 1);
    // 1.�����ٽ���
    taskENTER_CRITICAL();
    Inf_OLED_Refresh();
    // �˳��ٽ���
    taskEXIT_CRITICAL();
}

/**
 * @brief ��ʾ���Ĵ�����
 *
 */
void App_display_handle(void)
{
    // ��ʾ��غе�ѹֵ
    Inf_OLED_ShowString(0, 16, "V:", 16, 1);
    memset(display_buff, 0, 8);
    sprintf((char *)display_buff, "%.02f", read_PackVoltage);
    Inf_OLED_ShowString(16, 16, display_buff, 16, 1);

    // ��ʾ��ŵ�ĵ���ֵ
    Inf_OLED_ShowString(64, 16, "I:", 16, 1);
    memset(display_buff, 0, 8);
    sprintf((char *)display_buff, "%.02f", read_current);
    Inf_OLED_ShowString(80, 16, display_buff, 16, 1);

    // ��ʾ�¶�
    Inf_OLED_ShowString(0, 32, "T:", 16, 1);
    memset(display_buff, 0, 8);
    sprintf((char *)display_buff, "%.02f", temperature);
    Inf_OLED_ShowString(16, 32, display_buff, 16, 1);

    // ��ʾʣ�����
    Inf_OLED_ShowString(0, 48, "SOC:", 16, 1);
    memset(display_buff, 0, 8);
    sprintf((char *)display_buff, "%.02f", read_SOC);
    Inf_OLED_ShowString(32, 48, display_buff, 16, 1);

    // 1.�����ٽ���
    taskENTER_CRITICAL();
    Inf_OLED_Refresh();
    // �˳��ٽ���
    taskEXIT_CRITICAL();
}
