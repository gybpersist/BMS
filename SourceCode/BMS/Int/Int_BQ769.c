#include "Int_BQ769.h"

RegisterGroup bq769_reg;
uint16_t bq_gain = 0;   // 保存增益值 单位:μV
float bq_gain_mV = 0.0; // 保存增益值 单位:mV
int8_t bq_offset = 0;   // 保存偏移量 单位:mV

uint8_t BQ769_Read_Buff[128] = {0}; // I2C读取数据的缓冲区

// 剩余电量映射表
uint16_t SocOcvTab[101] =
    {
        3282,                                                       // 0%~1%
        3309, 3334, 3357, 3378, 3398, 3417, 3434, 3449, 3464, 3477, // 0%~10%
        3489, 3500, 3510, 3520, 3528, 3536, 3543, 3549, 3555, 3561, // 11%~20%
        3566, 3571, 3575, 3579, 3583, 3586, 3590, 3593, 3596, 3599, // 21%~30%
        3602, 3605, 3608, 3611, 3615, 3618, 3621, 3624, 3628, 3632, // 31%~40%
        3636, 3640, 3644, 3648, 3653, 3658, 3663, 3668, 3674, 3679, // 41%~50%
        3685, 3691, 3698, 3704, 3711, 3718, 3725, 3733, 3741, 3748, // 51%~60%
        3756, 3765, 3773, 3782, 3791, 3800, 3809, 3818, 3827, 3837, // 61%~70%
        3847, 3857, 3867, 3877, 3887, 3897, 3908, 3919, 3929, 3940, // 71%~80%
        3951, 3962, 3973, 3985, 3996, 4008, 4019, 4031, 4043, 4055, // 81%~90%
        4067, 4080, 4092, 4105, 4118, 4131, 4145, 4158, 4172, 4185, // 91~100%
};

/**
 * @brief BQ769芯片进行CRC校验
 *
 * @param data 要校验的数据
 * @param len 数据长度
 * @return uint8_t CRC校验后的数据
 */
static uint8_t BQ769x0_CRC8(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0x00;       // 初始值固定为0
    const uint8_t key = 0x07; // 文档强制指定!
    while (len--)
    {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ key; // key=0x07
            else
                crc <<= 1;
        }
    }
    return crc;
}

/**
 * @brief 向BQ769芯片的寄存器写入数据 (包含CRC校验)
 *
 * @param reg_addr 寄存器的地址
 * @param data 写入的数据
 */
static void Int_BQ769_WriteReg(uint16_t reg_addr, uint8_t data)
{
    // 1.准备要校验的数据
    uint8_t crc_buff[3] = {0};
    crc_buff[0] = BQ769_I2C_ADDRESS_WRITE;
    crc_buff[1] = (uint8_t)reg_addr;
    crc_buff[2] = data;
    // 进行crc校验
    uint8_t crc = BQ769x0_CRC8(crc_buff, 3);

    // 2.准备要写入的数据 (包含:data crc)
    uint8_t data_buff[2] = {0};
    data_buff[0] = data;
    data_buff[1] = crc;
    // 1.进入临界区
    taskENTER_CRITICAL();
    // 3.向BQ769芯片的寄存器写入数据 (包含CRC校验)
    HAL_I2C_Mem_Write(&hi2c2, BQ769_I2C_ADDRESS_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, data_buff, 2, HAL_MAX_DELAY);
    // 退出临界区
    taskEXIT_CRITICAL();
}

/**
 * @brief 向BQ769芯片的寄存器读取数据 (包含CRC校验)
 *
 * @param reg_addr 寄存器的地址
 * @param data 读取的数据
 * @param len 数据长度
 * @return uint8_t 0:成功 1:失败
 */
static uint8_t Int_BQ769_ReadReg(uint16_t reg_addr, uint8_t *data, uint16_t len)
{
    memset(BQ769_Read_Buff, 0, 128);
    // 1.进入临界区
    taskENTER_CRITICAL();
    // 1.先把数据读取出来放到BQ769_Read_Buff中
    HAL_I2C_Mem_Read(&hi2c2, BQ769_I2C_ADDRESS_READ, reg_addr, I2C_MEMADD_SIZE_8BIT, BQ769_Read_Buff, 2 * len, HAL_MAX_DELAY);
    // 退出临界区
    taskEXIT_CRITICAL();
    // 2.校验CRC  => 只对第一个字节的读取进行校验 => data:BQ769_Read_Buff[0]  crc:BQ769_Read_Buff[1]
    // 准备要校验的数据
    uint8_t crc_buff[2] = {0};
    crc_buff[0] = BQ769_I2C_ADDRESS_READ;
    crc_buff[1] = BQ769_Read_Buff[0];
    // 进行crc校验
    uint8_t crc = BQ769x0_CRC8(crc_buff, 2);

    // 3.两次crc校验的值进行对比,判断是否相同
    if (crc != BQ769_Read_Buff[1])
    {
        // debug_printfln("ReadReg error!!!");
        // 返回失败
        return 1;
    }

    // 4.获取读取到的数据
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = BQ769_Read_Buff[2 * i];
    }

    // 返回成功
    return 0;
}

/**
 * @brief 唤醒 BQ769芯片
 *
 */
static void Int_BQ769_WKUP(void)
{
    HAL_GPIO_WritePin(BQ_WK_UP_GPIO_Port, BQ_WK_UP_Pin, GPIO_PIN_SET);
    HAL_Delay(10); // 需要保持一段时间
    HAL_GPIO_WritePin(BQ_WK_UP_GPIO_Port, BQ_WK_UP_Pin, GPIO_PIN_RESET);
    HAL_Delay(1000); // 唤醒芯片到芯片的正常工作往往需要一段时间
}

/**
 * @brief 获取增益值
 *
 * @param gain 获取到的增益值
 * @note 单位: μV/位
 */
static void Int_BQ769_GetGAIN(uint16_t *gain)
{
    uint8_t gain1_data = 0;
    uint8_t gain2_data = 0;
    // 读取 ADCGain1寄存器
    Int_BQ769_ReadReg(BQ_ADCGAIN1, &gain1_data, 1);
    // 读取 ADCGain2寄存器
    Int_BQ769_ReadReg(BQ_ADCGAIN2, &gain2_data, 1);
    // 把gain1_data和gain2_data放到一起
    *gain = 365 + (((gain1_data << 1) & 0x18) | (gain2_data >> 5));
}

/**
 * @brief 获取偏移量
 *
 * @param offset 获取到的偏移量
 * @note 单位: mV
 */
static void Int_BQ769_GetOFFSET(int8_t *offset)
{
    uint8_t offset_data = 0;
    Int_BQ769_ReadReg(BQ_ADCOFFSET, &offset_data, 1);
    *offset = (int8_t)offset_data;
}

/**
 * @brief 查找剩余电量对应的下标 (电量 百分比形式)
 *
 * @param vol_mV 传入的电压(mV)
 * @return uint8_t 返回下标 (电量 百分比形式)
 */
static uint8_t Int_BQ769_GetSOCIndex(uint16_t vol_mV)
{
    uint8_t index = 0;
    if (vol_mV < SocOcvTab[0])
    {
        return 0;
    }
    if (vol_mV > SocOcvTab[100])
    {
        return 100;
    }

    while (vol_mV >= SocOcvTab[index])
    {
        index++;
    }
    return index;
}

/**
 * @brief 初始化 BQ769芯片 20
 *
 * @note 启动芯片,能够监视电池组的电压,电流,温度
 */
void Int_BQ769_Init(void)
{
    // 1 重启 : 先进入到船模式 ==> 再进入到正常模式
    // 1.1 先进入到船模式 [SHUTA:SHUTB]先写入00=>再写入01=>再写入10
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, 0x00); // 0000 0000
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, 0x01); // 0000 0001
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, 0x02); // 0000 0010
    HAL_Delay(1000);
    // 1.2 再进入到正常模式
    Int_BQ769_WKUP();

    // 2. 获取 增益值和偏移值
    // 2.1 获取增益值
    Int_BQ769_GetGAIN(&bq_gain);
    bq_gain_mV = bq_gain / 1000.0; // 单位换成mV
    // 2.2 获取偏移量
    Int_BQ769_GetOFFSET(&bq_offset);
    debug_printfln("gain:%dμV,offset:%dmV", bq_gain, bq_offset);

    // 3. 配置寄存器
    // 3.1 配置系统控制寄存器 SYS_CTRL
    bq769_reg.SysCtrl1.SysCtrl1Byte = 0x18; // ADC_EN:1 TEMP_SEL:1
    bq769_reg.SysCtrl2.SysCtrl2Byte = 0x42; // CC_EN:1   DSG_ON:1
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, bq769_reg.SysCtrl1.SysCtrl1Byte);
    Int_BQ769_WriteReg(BQ_SYS_CTRL2, bq769_reg.SysCtrl2.SysCtrl2Byte);
    debug_printfln("BQ_SYS_CTRL1:%02X,BQ_SYS_CTRL2:%02X", bq769_reg.SysCtrl1.SysCtrl1Byte, bq769_reg.SysCtrl2.SysCtrl2Byte);
    // 3.2 配置保护寄存器 PROTECT
    // 配置短路和过流的参数
    bq769_reg.Protect1.Protect1Bit.RSNS = 0;
    bq769_reg.Protect1.Protect1Bit.SCD_DELAY = BMS_SCD_DELAY_400us;
    bq769_reg.Protect1.Protect1Bit.SCD_THRESH = BMS_SCD_THRESH_200_100;
    Int_BQ769_WriteReg(BQ_PROTECT1, bq769_reg.Protect1.Protect1Byte);
    debug_printfln("BQ_PROTECT1:%02X", bq769_reg.Protect1.Protect1Byte);

    bq769_reg.Protect2.Protect2Bit.OCD_DELAY = BMS_OCD_DELAY_20ms;
    bq769_reg.Protect2.Protect2Bit.OCD_THRESH = BMS_OCD_THRESH_100_50;
    Int_BQ769_WriteReg(BQ_PROTECT2, bq769_reg.Protect2.Protect2Byte);
    debug_printfln("BQ_PROTECT2:%02X", bq769_reg.Protect2.Protect2Byte);
    // 配置欠压和过压的延迟
    bq769_reg.Protect3.Protect3Bit.UV_DELAY = BMS_UV_DELAY_4s;
    bq769_reg.Protect3.Protect3Bit.OV_DELAY = BMS_OV_DELAY_2s;
    Int_BQ769_WriteReg(BQ_PROTECT3, bq769_reg.Protect3.Protect3Byte);
    debug_printfln("BQ_PROTECT3:%02X", bq769_reg.Protect3.Protect3Byte);
    // 3.3 设置欠压(2.80)和过压(4.25)的值
    // 过压 写入的值是一个ADC的值  =>  需要经过计算才知道实际的电压值
    // 默认值是0xAC => 需要再次拼接前两位10 后四位1000 => 0x2AC8(10592)
    // => 10592 * 0.373 + 39 (mv) = 4.124V => 14位ADC * gain + offset = TLB_OV_PROTECT

    // 希望填写的过压值是 10<OVTrip>1000 => (TLB_OV_PROTECT - offset) / gain
    bq769_reg.OVTrip = (((uint16_t)((TLB_OV_PROTECT * 1000 - bq_offset) / bq_gain_mV)) >> 4) & 0xff;
    Int_BQ769_WriteReg(BQ_OV_TRIP, bq769_reg.OVTrip);
    debug_printfln("BQ_OV_TRIP:%02X", bq769_reg.OVTrip);

    // 欠压 写入的值是一个ADC的值  =>  需要经过计算才知道实际的电压值
    // 默认值是0x97 => 需要再次拼接前两位01 后四位0000 => 0x1970(6512)
    // => 6512 * 0.373 + 39 (mv) = 2.468V => 14位ADC * gain + offset = TLB_UV_PROTECT

    // 希望填写的欠压值是 10<UVTrip>1000 => (TLB_UV_PROTECT - offset) / gain
    bq769_reg.UVTrip = (((uint16_t)((TLB_UV_PROTECT * 1000 - bq_offset) / bq_gain_mV)) >> 4) & 0xff;
    Int_BQ769_WriteReg(BQ_UV_TRIP, bq769_reg.UVTrip);
    debug_printfln("BQ_UV_TRIP:%02X", bq769_reg.UVTrip);

    // 4. For optimal performance, these bits should be programmed to 0x19 upon device startup.
    bq769_reg.CCCfg = 0x19;
    Int_BQ769_WriteReg(BQ_CC_CFG, bq769_reg.CCCfg);
    debug_printfln("BQ_CC_CFG:%02X", bq769_reg.CCCfg);
}

/**
 * @brief 读取某个电池的电压值
 *
 * @param cell_index 电池的下标 [0 ~ 4]
 * @param voltage 读取到的电压值
 */
void Int_BQ769_GetCellVoltage(uint8_t cell_index, float *voltage)
{
    // 0x0C开始 VC1_HI VC1_LO ;VC2_HI VC2_LO ;VC3_HI VC3_LO ;VC4_HI VC4_LO ;VC5_HI VC5_LO ;
    if (cell_index >= CELL_NUM)
    {
        debug_printfln("cell_index error!!!");
        return;
    }

    // 计算对应电池的位置 并保存电压的高低位 (读取到的数据为 ADC的值,需要进行转换)
    uint8_t vc_addr_hi = BQ_VC1_HI + 2 * cell_index;
    uint8_t vc_addr_lo = BQ_VC1_LO + 2 * cell_index;
    uint8_t vc_hi_data = 0, vc_lo_data = 0;
    uint8_t err1 = Int_BQ769_ReadReg(vc_addr_hi, &vc_hi_data, 1);
    uint8_t err2 = Int_BQ769_ReadReg(vc_addr_lo, &vc_lo_data, 1);

    if (err1 == 0 && err2 == 0)
    {
        // 计算电压值 单位原为 mV (通过/1000.0) 转换为 V
        *voltage = (((vc_hi_data << 8) | vc_lo_data) * bq_gain_mV + bq_offset) / 1000.0;
    }
}

/**
 * @brief 读取充放电的电流值
 *
 * @param current 读取到的电流值
 */
void Int_BQ769_GetCurrent(float *current)
{
    // 使用库伦计数器读取特定电阻(5mV)的电压值
    uint8_t cc_hi_data = 0, cc_lo_data = 0;
    uint8_t err1 = Int_BQ769_ReadReg(BQ_CC_HI, &cc_hi_data, 1);
    uint8_t err2 = Int_BQ769_ReadReg(BQ_CC_LO, &cc_lo_data, 1);
    if (err1 != 0 || err2 != 0)
    {
        debug_printfln("GetCurrent error!!!");
        return;
    }

    // 获得完整的库伦计数器值(16位的CC读数)
    int16_t cc_adc = (cc_hi_data << 8) | cc_lo_data;

    // 限制读取的范围
    if (cc_adc > 32000 || cc_adc < -32000)
    {
        debug_printfln("cc_adc error\n");
        return;
    }

    // 保留绝对值
    if (cc_adc < 0)
    {
        cc_adc = -cc_adc;
    }

    // 将16位的CC读数转换为模拟电压 (uV)
    // 公式: 模拟电压(uV) = 16位的CC读数 * 8.44 uV/位
    float cc_voltage = cc_adc * 8.44;

    // 计算充放电的电流值
    // 公式: I = V / R ==> I = uV / uΩ
    *current = cc_voltage / TLB_CURRENT_SENSE_RESISTANCE / 1000.0;
}

/**
 * @brief 读取电池盒的总电压
 *
 * @param voltage 读取到的电池盒的总电压
 */
void Int_BQ769_GetPackVoltage(float *voltage)
{
    // 读取BAT寄存器的高低位
    uint8_t bat_hi_data = 0, bat_lo_data = 0;
    uint8_t err1 = Int_BQ769_ReadReg(BQ_BAT_HI, &bat_hi_data, 1);
    uint8_t err2 = Int_BQ769_ReadReg(BQ_BAT_LO, &bat_lo_data, 1);
    if (err1 != 0 || err2 != 0)
    {
        debug_printfln("GetPackVoltage error!!!");
        return;
    }

    // 获得完整bat数据
    int16_t bat_adc = (bat_hi_data << 8) | bat_lo_data;

    // 将读数转换为模拟电压 (通过/1000.0) 转换为 V
    // 公式: Vbat = 4 * gain(mV) * bat_adc + (电池数量 * offset)
    *voltage = ((4 * bq_gain_mV * bat_adc) + CELL_NUM * bq_offset) / 1000.0;
}

/**
 * @brief 读取温度值
 *
 * @param temperature 读取到的温度值
 */
void Int_BQ769_GetTemperature(float *temperature)
{
    // 读取TS1寄存器的高低位 (BQ76920只有一个热敏电阻,所以只能使用TS1寄存器)
    uint8_t ts1_hi_data = 0, ts1_lo_data = 0;
    uint8_t err1 = Int_BQ769_ReadReg(BQ_TS1_HI, &ts1_hi_data, 1);
    uint8_t err2 = Int_BQ769_ReadReg(BQ_TS1_LO, &ts1_lo_data, 1);
    if (err1 != 0 || err2 != 0)
    {
        debug_printfln("GetTemperature error!!!");
        return;
    }

    // 获得完整ts1数据 (14位ADC数据)
    uint16_t ts1_adc = ((ts1_hi_data & 0x3f) << 8) | ts1_lo_data;

    // 计算特定位置(热敏电阻)的电压值 mV
    // 公式: Vtsx = ts1_adc(十进制表示的) * 382 uV / 1000.0
    float Vtsx = ts1_adc * 382 / 1000.0;
    // 计算特定位置(热敏电阻)的电阻值 Ω
    // 公式: Rts = (10000.0 * Vtsx) / (3.3 * 1000 - Vtsx)
    float Rts = (10000.0 * Vtsx) / (3.3 * 1000 - Vtsx);

    // 使用热敏电阻的电阻值计算出温度值
    // 1 / temperature = log(热敏电阻当前阻值 / 10000.0) * (1 / B值) + 1 / (温度T0(25°C=298.15K))
    float tmp = log(Rts / 10000.0) * (1 / TLB_THERMISTOR_B) + 1 / (25 + 273.15);
    *temperature = 1 / tmp - 273.15;
}

/**
 * @brief 获取电池盒的剩余电量
 *
 * @param cells_voltage 电池的数组
 * @param soc 获取到的剩余电量
 * @param temp 传入的温度
 */
void Int_BQ769_GetSOC(float cells_voltage[], float *soc, float temp)
{
    // 创建保存下标的数组
    uint8_t index[CELL_NUM] = {0};
    // 保存总电量
    uint16_t index_sum = 0;

    // 循环获取每个电池的剩余电量
    for (uint8_t i = 0; i < CELL_NUM; i++)
    {
        // 查找剩余电量对应的下标 (电量 百分比形式)
        index[i] = Int_BQ769_GetSOCIndex(cells_voltage[i] * 1000);
        index_sum += index[i]; // 计算求和
    }
    debug_printfln("index:%d %d %d %d %d", index[0], index[1], index[2], index[3], index[4]);

    // 添加温度补偿 => 标准流程是单独的电池单独测量温度 进行温度补偿之后求平均值作为电量
    // 因为没有单独测量温度 => 直接使用平均值做温度补偿
    // 计算剩余电压的平均电压
    float index_average = index_sum * 1.0 / CELL_NUM;
    // 公式: SOCadj = SOCtable * [1 + Kt * (T - 25)]
    // Kt:(-0.5%/°C)温度补偿系数 T:当前电池温度    T>40°C:附加-0.3%/°C高温衰减
    if (temp <= 40)
    {
        *soc = index_average * (1 - 0.005 * (temp - 25));
    }
    if (temp > 40)
    {
        *soc = index_average * (1 - 0.005 * 40 - 0.008 * (temp - 40));
    }
}

/**
 * @brief 是否开启充电控制
 *
 * @param ctrl 1:开启 0:不开启
 */
void Int_BQ769_ChargeCtrl(uint8_t ctrl)
{
    if (ctrl == 1)
    {
        bq769_reg.SysCtrl2.SysCtrl2Bit.CHG_ON = 1;
    }
    else
    {
        bq769_reg.SysCtrl2.SysCtrl2Bit.CHG_ON = 0;
    }
    debug_printfln("CHG:%02x", bq769_reg.SysCtrl2.SysCtrl2Byte);
    Int_BQ769_WriteReg(BQ_SYS_CTRL2, bq769_reg.SysCtrl2.SysCtrl2Byte);
}

/**
 * @brief 是否开启放电控制
 *
 * @param ctrl 1:开启 0:不开启
 */
void Int_BQ769_DisChargeCtrl(uint8_t ctrl)
{
    if (ctrl == 1)
    {
        bq769_reg.SysCtrl2.SysCtrl2Bit.DSG_ON = 1;
    }
    else
    {
        bq769_reg.SysCtrl2.SysCtrl2Bit.DSG_ON = 0;
    }
    debug_printfln("DSG:%02x", bq769_reg.SysCtrl2.SysCtrl2Byte);
    Int_BQ769_WriteReg(BQ_SYS_CTRL2, bq769_reg.SysCtrl2.SysCtrl2Byte);
}
