#include "Int_BQ769.h"

RegisterGroup bq769_reg;
uint16_t bq_gain = 0;   // ��������ֵ ��λ:��V
float bq_gain_mV = 0.0; // ��������ֵ ��λ:mV
int8_t bq_offset = 0;   // ����ƫ���� ��λ:mV

uint8_t BQ769_Read_Buff[128] = {0}; // I2C��ȡ���ݵĻ�����

// ʣ�����ӳ���
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
 * @brief BQ769оƬ����CRCУ��
 *
 * @param data ҪУ�������
 * @param len ���ݳ���
 * @return uint8_t CRCУ��������
 */
static uint8_t BQ769x0_CRC8(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0x00;       // ��ʼֵ�̶�Ϊ0
    const uint8_t key = 0x07; // �ĵ�ǿ��ָ��!
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
 * @brief ��BQ769оƬ�ļĴ���д������ (����CRCУ��)
 *
 * @param reg_addr �Ĵ����ĵ�ַ
 * @param data д�������
 */
static void Int_BQ769_WriteReg(uint16_t reg_addr, uint8_t data)
{
    // 1.׼��ҪУ�������
    uint8_t crc_buff[3] = {0};
    crc_buff[0] = BQ769_I2C_ADDRESS_WRITE;
    crc_buff[1] = (uint8_t)reg_addr;
    crc_buff[2] = data;
    // ����crcУ��
    uint8_t crc = BQ769x0_CRC8(crc_buff, 3);

    // 2.׼��Ҫд������� (����:data crc)
    uint8_t data_buff[2] = {0};
    data_buff[0] = data;
    data_buff[1] = crc;
    // 1.�����ٽ���
    taskENTER_CRITICAL();
    // 3.��BQ769оƬ�ļĴ���д������ (����CRCУ��)
    HAL_I2C_Mem_Write(&hi2c2, BQ769_I2C_ADDRESS_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, data_buff, 2, HAL_MAX_DELAY);
    // �˳��ٽ���
    taskEXIT_CRITICAL();
}

/**
 * @brief ��BQ769оƬ�ļĴ�����ȡ���� (����CRCУ��)
 *
 * @param reg_addr �Ĵ����ĵ�ַ
 * @param data ��ȡ������
 * @param len ���ݳ���
 * @return uint8_t 0:�ɹ� 1:ʧ��
 */
static uint8_t Int_BQ769_ReadReg(uint16_t reg_addr, uint8_t *data, uint16_t len)
{
    memset(BQ769_Read_Buff, 0, 128);
    // 1.�����ٽ���
    taskENTER_CRITICAL();
    // 1.�Ȱ����ݶ�ȡ�����ŵ�BQ769_Read_Buff��
    HAL_I2C_Mem_Read(&hi2c2, BQ769_I2C_ADDRESS_READ, reg_addr, I2C_MEMADD_SIZE_8BIT, BQ769_Read_Buff, 2 * len, HAL_MAX_DELAY);
    // �˳��ٽ���
    taskEXIT_CRITICAL();
    // 2.У��CRC  => ֻ�Ե�һ���ֽڵĶ�ȡ����У�� => data:BQ769_Read_Buff[0]  crc:BQ769_Read_Buff[1]
    // ׼��ҪУ�������
    uint8_t crc_buff[2] = {0};
    crc_buff[0] = BQ769_I2C_ADDRESS_READ;
    crc_buff[1] = BQ769_Read_Buff[0];
    // ����crcУ��
    uint8_t crc = BQ769x0_CRC8(crc_buff, 2);

    // 3.����crcУ���ֵ���жԱ�,�ж��Ƿ���ͬ
    if (crc != BQ769_Read_Buff[1])
    {
        // debug_printfln("ReadReg error!!!");
        // ����ʧ��
        return 1;
    }

    // 4.��ȡ��ȡ��������
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = BQ769_Read_Buff[2 * i];
    }

    // ���سɹ�
    return 0;
}

/**
 * @brief ���� BQ769оƬ
 *
 */
static void Int_BQ769_WKUP(void)
{
    HAL_GPIO_WritePin(BQ_WK_UP_GPIO_Port, BQ_WK_UP_Pin, GPIO_PIN_SET);
    HAL_Delay(10); // ��Ҫ����һ��ʱ��
    HAL_GPIO_WritePin(BQ_WK_UP_GPIO_Port, BQ_WK_UP_Pin, GPIO_PIN_RESET);
    HAL_Delay(1000); // ����оƬ��оƬ����������������Ҫһ��ʱ��
}

/**
 * @brief ��ȡ����ֵ
 *
 * @param gain ��ȡ��������ֵ
 * @note ��λ: ��V/λ
 */
static void Int_BQ769_GetGAIN(uint16_t *gain)
{
    uint8_t gain1_data = 0;
    uint8_t gain2_data = 0;
    // ��ȡ ADCGain1�Ĵ���
    Int_BQ769_ReadReg(BQ_ADCGAIN1, &gain1_data, 1);
    // ��ȡ ADCGain2�Ĵ���
    Int_BQ769_ReadReg(BQ_ADCGAIN2, &gain2_data, 1);
    // ��gain1_data��gain2_data�ŵ�һ��
    *gain = 365 + (((gain1_data << 1) & 0x18) | (gain2_data >> 5));
}

/**
 * @brief ��ȡƫ����
 *
 * @param offset ��ȡ����ƫ����
 * @note ��λ: mV
 */
static void Int_BQ769_GetOFFSET(int8_t *offset)
{
    uint8_t offset_data = 0;
    Int_BQ769_ReadReg(BQ_ADCOFFSET, &offset_data, 1);
    *offset = (int8_t)offset_data;
}

/**
 * @brief ����ʣ�������Ӧ���±� (���� �ٷֱ���ʽ)
 *
 * @param vol_mV ����ĵ�ѹ(mV)
 * @return uint8_t �����±� (���� �ٷֱ���ʽ)
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
 * @brief ��ʼ�� BQ769оƬ 20
 *
 * @note ����оƬ,�ܹ����ӵ����ĵ�ѹ,����,�¶�
 */
void Int_BQ769_Init(void)
{
    // 1 ���� : �Ƚ��뵽��ģʽ ==> �ٽ��뵽����ģʽ
    // 1.1 �Ƚ��뵽��ģʽ [SHUTA:SHUTB]��д��00=>��д��01=>��д��10
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, 0x00); // 0000 0000
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, 0x01); // 0000 0001
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, 0x02); // 0000 0010
    HAL_Delay(1000);
    // 1.2 �ٽ��뵽����ģʽ
    Int_BQ769_WKUP();

    // 2. ��ȡ ����ֵ��ƫ��ֵ
    // 2.1 ��ȡ����ֵ
    Int_BQ769_GetGAIN(&bq_gain);
    bq_gain_mV = bq_gain / 1000.0; // ��λ����mV
    // 2.2 ��ȡƫ����
    Int_BQ769_GetOFFSET(&bq_offset);
    debug_printfln("gain:%d��V,offset:%dmV", bq_gain, bq_offset);

    // 3. ���üĴ���
    // 3.1 ����ϵͳ���ƼĴ��� SYS_CTRL
    bq769_reg.SysCtrl1.SysCtrl1Byte = 0x18; // ADC_EN:1 TEMP_SEL:1
    bq769_reg.SysCtrl2.SysCtrl2Byte = 0x42; // CC_EN:1   DSG_ON:1
    Int_BQ769_WriteReg(BQ_SYS_CTRL1, bq769_reg.SysCtrl1.SysCtrl1Byte);
    Int_BQ769_WriteReg(BQ_SYS_CTRL2, bq769_reg.SysCtrl2.SysCtrl2Byte);
    debug_printfln("BQ_SYS_CTRL1:%02X,BQ_SYS_CTRL2:%02X", bq769_reg.SysCtrl1.SysCtrl1Byte, bq769_reg.SysCtrl2.SysCtrl2Byte);
    // 3.2 ���ñ����Ĵ��� PROTECT
    // ���ö�·�͹����Ĳ���
    bq769_reg.Protect1.Protect1Bit.RSNS = 0;
    bq769_reg.Protect1.Protect1Bit.SCD_DELAY = BMS_SCD_DELAY_400us;
    bq769_reg.Protect1.Protect1Bit.SCD_THRESH = BMS_SCD_THRESH_200_100;
    Int_BQ769_WriteReg(BQ_PROTECT1, bq769_reg.Protect1.Protect1Byte);
    debug_printfln("BQ_PROTECT1:%02X", bq769_reg.Protect1.Protect1Byte);

    bq769_reg.Protect2.Protect2Bit.OCD_DELAY = BMS_OCD_DELAY_20ms;
    bq769_reg.Protect2.Protect2Bit.OCD_THRESH = BMS_OCD_THRESH_100_50;
    Int_BQ769_WriteReg(BQ_PROTECT2, bq769_reg.Protect2.Protect2Byte);
    debug_printfln("BQ_PROTECT2:%02X", bq769_reg.Protect2.Protect2Byte);
    // ����Ƿѹ�͹�ѹ���ӳ�
    bq769_reg.Protect3.Protect3Bit.UV_DELAY = BMS_UV_DELAY_4s;
    bq769_reg.Protect3.Protect3Bit.OV_DELAY = BMS_OV_DELAY_2s;
    Int_BQ769_WriteReg(BQ_PROTECT3, bq769_reg.Protect3.Protect3Byte);
    debug_printfln("BQ_PROTECT3:%02X", bq769_reg.Protect3.Protect3Byte);
    // 3.3 ����Ƿѹ(2.80)�͹�ѹ(4.25)��ֵ
    // ��ѹ д���ֵ��һ��ADC��ֵ  =>  ��Ҫ���������֪��ʵ�ʵĵ�ѹֵ
    // Ĭ��ֵ��0xAC => ��Ҫ�ٴ�ƴ��ǰ��λ10 ����λ1000 => 0x2AC8(10592)
    // => 10592 * 0.373 + 39 (mv) = 4.124V => 14λADC * gain + offset = TLB_OV_PROTECT

    // ϣ����д�Ĺ�ѹֵ�� 10<OVTrip>1000 => (TLB_OV_PROTECT - offset) / gain
    bq769_reg.OVTrip = (((uint16_t)((TLB_OV_PROTECT * 1000 - bq_offset) / bq_gain_mV)) >> 4) & 0xff;
    Int_BQ769_WriteReg(BQ_OV_TRIP, bq769_reg.OVTrip);
    debug_printfln("BQ_OV_TRIP:%02X", bq769_reg.OVTrip);

    // Ƿѹ д���ֵ��һ��ADC��ֵ  =>  ��Ҫ���������֪��ʵ�ʵĵ�ѹֵ
    // Ĭ��ֵ��0x97 => ��Ҫ�ٴ�ƴ��ǰ��λ01 ����λ0000 => 0x1970(6512)
    // => 6512 * 0.373 + 39 (mv) = 2.468V => 14λADC * gain + offset = TLB_UV_PROTECT

    // ϣ����д��Ƿѹֵ�� 10<UVTrip>1000 => (TLB_UV_PROTECT - offset) / gain
    bq769_reg.UVTrip = (((uint16_t)((TLB_UV_PROTECT * 1000 - bq_offset) / bq_gain_mV)) >> 4) & 0xff;
    Int_BQ769_WriteReg(BQ_UV_TRIP, bq769_reg.UVTrip);
    debug_printfln("BQ_UV_TRIP:%02X", bq769_reg.UVTrip);

    // 4. For optimal performance, these bits should be programmed to 0x19 upon device startup.
    bq769_reg.CCCfg = 0x19;
    Int_BQ769_WriteReg(BQ_CC_CFG, bq769_reg.CCCfg);
    debug_printfln("BQ_CC_CFG:%02X", bq769_reg.CCCfg);
}

/**
 * @brief ��ȡĳ����صĵ�ѹֵ
 *
 * @param cell_index ��ص��±� [0 ~ 4]
 * @param voltage ��ȡ���ĵ�ѹֵ
 */
void Int_BQ769_GetCellVoltage(uint8_t cell_index, float *voltage)
{
    // 0x0C��ʼ VC1_HI VC1_LO ;VC2_HI VC2_LO ;VC3_HI VC3_LO ;VC4_HI VC4_LO ;VC5_HI VC5_LO ;
    if (cell_index >= CELL_NUM)
    {
        debug_printfln("cell_index error!!!");
        return;
    }

    // �����Ӧ��ص�λ�� �������ѹ�ĸߵ�λ (��ȡ��������Ϊ ADC��ֵ,��Ҫ����ת��)
    uint8_t vc_addr_hi = BQ_VC1_HI + 2 * cell_index;
    uint8_t vc_addr_lo = BQ_VC1_LO + 2 * cell_index;
    uint8_t vc_hi_data = 0, vc_lo_data = 0;
    uint8_t err1 = Int_BQ769_ReadReg(vc_addr_hi, &vc_hi_data, 1);
    uint8_t err2 = Int_BQ769_ReadReg(vc_addr_lo, &vc_lo_data, 1);

    if (err1 == 0 && err2 == 0)
    {
        // �����ѹֵ ��λԭΪ mV (ͨ��/1000.0) ת��Ϊ V
        *voltage = (((vc_hi_data << 8) | vc_lo_data) * bq_gain_mV + bq_offset) / 1000.0;
    }
}

/**
 * @brief ��ȡ��ŵ�ĵ���ֵ
 *
 * @param current ��ȡ���ĵ���ֵ
 */
void Int_BQ769_GetCurrent(float *current)
{
    // ʹ�ÿ��׼�������ȡ�ض�����(5mV)�ĵ�ѹֵ
    uint8_t cc_hi_data = 0, cc_lo_data = 0;
    uint8_t err1 = Int_BQ769_ReadReg(BQ_CC_HI, &cc_hi_data, 1);
    uint8_t err2 = Int_BQ769_ReadReg(BQ_CC_LO, &cc_lo_data, 1);
    if (err1 != 0 || err2 != 0)
    {
        debug_printfln("GetCurrent error!!!");
        return;
    }

    // ��������Ŀ��׼�����ֵ(16λ��CC����)
    int16_t cc_adc = (cc_hi_data << 8) | cc_lo_data;

    // ���ƶ�ȡ�ķ�Χ
    if (cc_adc > 32000 || cc_adc < -32000)
    {
        debug_printfln("cc_adc error\n");
        return;
    }

    // ��������ֵ
    if (cc_adc < 0)
    {
        cc_adc = -cc_adc;
    }

    // ��16λ��CC����ת��Ϊģ���ѹ (uV)
    // ��ʽ: ģ���ѹ(uV) = 16λ��CC���� * 8.44 uV/λ
    float cc_voltage = cc_adc * 8.44;

    // �����ŵ�ĵ���ֵ
    // ��ʽ: I = V / R ==> I = uV / u��
    *current = cc_voltage / TLB_CURRENT_SENSE_RESISTANCE / 1000.0;
}

/**
 * @brief ��ȡ��غе��ܵ�ѹ
 *
 * @param voltage ��ȡ���ĵ�غе��ܵ�ѹ
 */
void Int_BQ769_GetPackVoltage(float *voltage)
{
    // ��ȡBAT�Ĵ����ĸߵ�λ
    uint8_t bat_hi_data = 0, bat_lo_data = 0;
    uint8_t err1 = Int_BQ769_ReadReg(BQ_BAT_HI, &bat_hi_data, 1);
    uint8_t err2 = Int_BQ769_ReadReg(BQ_BAT_LO, &bat_lo_data, 1);
    if (err1 != 0 || err2 != 0)
    {
        debug_printfln("GetPackVoltage error!!!");
        return;
    }

    // �������bat����
    int16_t bat_adc = (bat_hi_data << 8) | bat_lo_data;

    // ������ת��Ϊģ���ѹ (ͨ��/1000.0) ת��Ϊ V
    // ��ʽ: Vbat = 4 * gain(mV) * bat_adc + (������� * offset)
    *voltage = ((4 * bq_gain_mV * bat_adc) + CELL_NUM * bq_offset) / 1000.0;
}

/**
 * @brief ��ȡ�¶�ֵ
 *
 * @param temperature ��ȡ�����¶�ֵ
 */
void Int_BQ769_GetTemperature(float *temperature)
{
    // ��ȡTS1�Ĵ����ĸߵ�λ (BQ76920ֻ��һ����������,����ֻ��ʹ��TS1�Ĵ���)
    uint8_t ts1_hi_data = 0, ts1_lo_data = 0;
    uint8_t err1 = Int_BQ769_ReadReg(BQ_TS1_HI, &ts1_hi_data, 1);
    uint8_t err2 = Int_BQ769_ReadReg(BQ_TS1_LO, &ts1_lo_data, 1);
    if (err1 != 0 || err2 != 0)
    {
        debug_printfln("GetTemperature error!!!");
        return;
    }

    // �������ts1���� (14λADC����)
    uint16_t ts1_adc = ((ts1_hi_data & 0x3f) << 8) | ts1_lo_data;

    // �����ض�λ��(��������)�ĵ�ѹֵ mV
    // ��ʽ: Vtsx = ts1_adc(ʮ���Ʊ�ʾ��) * 382 uV / 1000.0
    float Vtsx = ts1_adc * 382 / 1000.0;
    // �����ض�λ��(��������)�ĵ���ֵ ��
    // ��ʽ: Rts = (10000.0 * Vtsx) / (3.3 * 1000 - Vtsx)
    float Rts = (10000.0 * Vtsx) / (3.3 * 1000 - Vtsx);

    // ʹ����������ĵ���ֵ������¶�ֵ
    // 1 / temperature = log(�������赱ǰ��ֵ / 10000.0) * (1 / Bֵ) + 1 / (�¶�T0(25��C=298.15K))
    float tmp = log(Rts / 10000.0) * (1 / TLB_THERMISTOR_B) + 1 / (25 + 273.15);
    *temperature = 1 / tmp - 273.15;
}

/**
 * @brief ��ȡ��غе�ʣ�����
 *
 * @param cells_voltage ��ص�����
 * @param soc ��ȡ����ʣ�����
 * @param temp ������¶�
 */
void Int_BQ769_GetSOC(float cells_voltage[], float *soc, float temp)
{
    // ���������±������
    uint8_t index[CELL_NUM] = {0};
    // �����ܵ���
    uint16_t index_sum = 0;

    // ѭ����ȡÿ����ص�ʣ�����
    for (uint8_t i = 0; i < CELL_NUM; i++)
    {
        // ����ʣ�������Ӧ���±� (���� �ٷֱ���ʽ)
        index[i] = Int_BQ769_GetSOCIndex(cells_voltage[i] * 1000);
        index_sum += index[i]; // �������
    }
    debug_printfln("index:%d %d %d %d %d", index[0], index[1], index[2], index[3], index[4]);

    // ����¶Ȳ��� => ��׼�����ǵ����ĵ�ص��������¶� �����¶Ȳ���֮����ƽ��ֵ��Ϊ����
    // ��Ϊû�е��������¶� => ֱ��ʹ��ƽ��ֵ���¶Ȳ���
    // ����ʣ���ѹ��ƽ����ѹ
    float index_average = index_sum * 1.0 / CELL_NUM;
    // ��ʽ: SOCadj = SOCtable * [1 + Kt * (T - 25)]
    // Kt:(-0.5%/��C)�¶Ȳ���ϵ�� T:��ǰ����¶�    T>40��C:����-0.3%/��C����˥��
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
 * @brief �Ƿ���������
 *
 * @param ctrl 1:���� 0:������
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
 * @brief �Ƿ����ŵ����
 *
 * @param ctrl 1:���� 0:������
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
