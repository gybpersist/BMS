#ifndef __APP_DISPLAY_H__
#define __APP_DISPLAY_H__

#include "Int_OLED.h"
#include "Int_BQ769.h"
#include "stdio.h"

// 保存读取到的电池电压
extern float read_CellVoltage_buff[5];
// 保存读取到的电流值
extern float read_current;
// 保存读取到的电池盒的电压值
extern float read_PackVoltage;
// 保存读取到的温度
extern float temperature;
// 保存读取到的剩余电量
extern float read_SOC;


/**
 * @brief 显示屏初始化
 * 
 */
void App_display_Init(void);

/**
 * @brief 显示屏的处理函数
 * 
 */
void App_display_handle(void);

#endif /* __APP_DISPLAY_H__ */
