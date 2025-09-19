#include "App_display.h"

uint8_t display_buff[8];

/**
 * @brief 显示屏初始化
 *
 */
void App_display_Init(void)
{
    Inf_OLED_Init();
    Inf_OLED_ShowString(32, 0, "BMS", 16, 1);
    Inf_OLED_ShowChinese(56, 0, 5, 16, 1);
    Inf_OLED_ShowChinese(72, 0, 6, 16, 1);
    // 1.进入临界区
    taskENTER_CRITICAL();
    Inf_OLED_Refresh();
    // 退出临界区
    taskEXIT_CRITICAL();
}

/**
 * @brief 显示屏的处理函数
 *
 */
void App_display_handle(void)
{
    // 显示电池盒电压值
    Inf_OLED_ShowString(0, 16, "V:", 16, 1);
    memset(display_buff, 0, 8);
    sprintf((char *)display_buff, "%.02f", read_PackVoltage);
    Inf_OLED_ShowString(16, 16, display_buff, 16, 1);

    // 显示充放电的电流值
    Inf_OLED_ShowString(64, 16, "I:", 16, 1);
    memset(display_buff, 0, 8);
    sprintf((char *)display_buff, "%.02f", read_current);
    Inf_OLED_ShowString(80, 16, display_buff, 16, 1);

    // 显示温度
    Inf_OLED_ShowString(0, 32, "T:", 16, 1);
    memset(display_buff, 0, 8);
    sprintf((char *)display_buff, "%.02f", temperature);
    Inf_OLED_ShowString(16, 32, display_buff, 16, 1);

    // 显示剩余电量
    Inf_OLED_ShowString(0, 48, "SOC:", 16, 1);
    memset(display_buff, 0, 8);
    sprintf((char *)display_buff, "%.02f", read_SOC);
    Inf_OLED_ShowString(32, 48, display_buff, 16, 1);

    // 1.进入临界区
    taskENTER_CRITICAL();
    Inf_OLED_Refresh();
    // 退出临界区
    taskEXIT_CRITICAL();
}
