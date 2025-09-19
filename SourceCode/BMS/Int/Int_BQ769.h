#ifndef __INT_BQ769_H__
#define __INT_BQ769_H__

#include "i2c.h"
#include "Inf_BQ769_BSP.h"
#include "Com_debug.h"
#include "math.h"
#include "FreeRTOS.h"
#include "task.h"

#define BQ769_I2C_ADDRESS 0x08       // bq769��i2c��ַ
#define BQ769_I2C_ADDRESS_WRITE 0x10 // bq769��д��ַ
#define BQ769_I2C_ADDRESS_READ 0x11  // bq769�Ķ���ַ

#define CELL_NUM 5 // �������

// ��Ԫ﮵�� �ĵ�ѹֵ V
#define TLB_OV_PROTECT 4.25       // �����ѹ������ѹ
#define TLB_OV_RELIEVE 4.10       // �����ѹ�ָ���ѹ
#define TLB_UV_PROTECT 2.80       // ����Ƿѹ������ѹ
#define TLB_UV_RELIEVE 3.00       // ����Ƿѹ�ָ���ѹ
#define TLB_SHUTDOWN_VOLTAGE 2.80 // �Զ��ػ���ѹ
#define TLB_BALANCE_VOLTAGE 3.30  // ������ʼ��ѹ

// ��������λ�õĵ���ֵ��5m��
#define TLB_CURRENT_SENSE_RESISTANCE 5.0

// ʹ����������Ĳ���(Bֵ)
#define TLB_THERMISTOR_B 3950.0

/**
 * @brief ��ʼ�� BQ769оƬ 20
 *
 * @note ����оƬ,�ܹ����ӵ����ĵ�ѹ,����,�¶�
 */
void Int_BQ769_Init(void);

/**
 * @brief ��ȡĳ����صĵ�ѹֵ
 *
 * @param cell_index ��ص��±� [0 ~ 4]
 * @param voltage ��ȡ���ĵ�ѹֵ
 */
void Int_BQ769_GetCellVoltage(uint8_t cell_index, float *voltage);

/**
 * @brief ��ȡ��ŵ�ĵ���ֵ
 *
 * @param current ��ȡ���ĵ���ֵ
 */
void Int_BQ769_GetCurrent(float *current);

/**
 * @brief ��ȡ��غе��ܵ�ѹ
 *
 * @param voltage ��ȡ���ĵ�غе��ܵ�ѹ
 */
void Int_BQ769_GetPackVoltage(float *voltage);

/**
 * @brief ��ȡ�¶�ֵ
 *
 * @param temperature ��ȡ�����¶�ֵ
 */
void Int_BQ769_GetTemperature(float *temperature);

/**
 * @brief ��ȡ��غе�ʣ�����
 *
 * @param cells_voltage ��ص�����
 * @param soc ��ȡ����ʣ�����
 * @param temp ������¶�
 */
void Int_BQ769_GetSOC(float cells_voltage[], float *soc, float temp);

/**
 * @brief �Ƿ���������
 *
 * @param ctrl 1:���� 0:������
 */
void Int_BQ769_ChargeCtrl(uint8_t ctrl);

/**
 * @brief �Ƿ����ŵ����
 *
 * @param ctrl 1:���� 0:������
 */
void Int_BQ769_DisChargeCtrl(uint8_t ctrl);

#endif /* __INT_BQ769_H__ */
