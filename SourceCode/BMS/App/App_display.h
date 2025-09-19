#ifndef __APP_DISPLAY_H__
#define __APP_DISPLAY_H__

#include "Int_OLED.h"
#include "Int_BQ769.h"
#include "stdio.h"

// �����ȡ���ĵ�ص�ѹ
extern float read_CellVoltage_buff[5];
// �����ȡ���ĵ���ֵ
extern float read_current;
// �����ȡ���ĵ�غеĵ�ѹֵ
extern float read_PackVoltage;
// �����ȡ�����¶�
extern float temperature;
// �����ȡ����ʣ�����
extern float read_SOC;


/**
 * @brief ��ʾ����ʼ��
 * 
 */
void App_display_Init(void);

/**
 * @brief ��ʾ���Ĵ�����
 * 
 */
void App_display_handle(void);

#endif /* __APP_DISPLAY_H__ */
