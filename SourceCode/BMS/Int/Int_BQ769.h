#ifndef __INT_BQ769_H__
#define __INT_BQ769_H__

#include "i2c.h"
#include "Inf_BQ769_BSP.h"
#include "Com_debug.h"
#include "math.h"
#include "FreeRTOS.h"
#include "task.h"

#define BQ769_I2C_ADDRESS 0x08       // bq769的i2c地址
#define BQ769_I2C_ADDRESS_WRITE 0x10 // bq769的写地址
#define BQ769_I2C_ADDRESS_READ 0x11  // bq769的读地址

#define CELL_NUM 5 // 电池数量

// 三元锂电池 的电压值 V
#define TLB_OV_PROTECT 4.25       // 单体过压保护电压
#define TLB_OV_RELIEVE 4.10       // 单体过压恢复电压
#define TLB_UV_PROTECT 2.80       // 单体欠压保护电压
#define TLB_UV_RELIEVE 3.00       // 单体欠压恢复电压
#define TLB_SHUTDOWN_VOLTAGE 2.80 // 自动关机电压
#define TLB_BALANCE_VOLTAGE 3.30  // 均衡起始电压

// 测量电流位置的电阻值是5mΩ
#define TLB_CURRENT_SENSE_RESISTANCE 5.0

// 使用热敏电阻的参数(B值)
#define TLB_THERMISTOR_B 3950.0

/**
 * @brief 初始化 BQ769芯片 20
 *
 * @note 启动芯片,能够监视电池组的电压,电流,温度
 */
void Int_BQ769_Init(void);

/**
 * @brief 读取某个电池的电压值
 *
 * @param cell_index 电池的下标 [0 ~ 4]
 * @param voltage 读取到的电压值
 */
void Int_BQ769_GetCellVoltage(uint8_t cell_index, float *voltage);

/**
 * @brief 读取充放电的电流值
 *
 * @param current 读取到的电流值
 */
void Int_BQ769_GetCurrent(float *current);

/**
 * @brief 读取电池盒的总电压
 *
 * @param voltage 读取到的电池盒的总电压
 */
void Int_BQ769_GetPackVoltage(float *voltage);

/**
 * @brief 读取温度值
 *
 * @param temperature 读取到的温度值
 */
void Int_BQ769_GetTemperature(float *temperature);

/**
 * @brief 获取电池盒的剩余电量
 *
 * @param cells_voltage 电池的数组
 * @param soc 获取到的剩余电量
 * @param temp 传入的温度
 */
void Int_BQ769_GetSOC(float cells_voltage[], float *soc, float temp);

/**
 * @brief 是否开启充电控制
 *
 * @param ctrl 1:开启 0:不开启
 */
void Int_BQ769_ChargeCtrl(uint8_t ctrl);

/**
 * @brief 是否开启放电控制
 *
 * @param ctrl 1:开启 0:不开启
 */
void Int_BQ769_DisChargeCtrl(uint8_t ctrl);

#endif /* __INT_BQ769_H__ */
