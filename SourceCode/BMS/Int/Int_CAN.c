#include "Int_CAN.h"

/**
 * @brief CAN 通信初始化
 *
 */
void Int_CAN_Init(void)
{
    // 配置过滤器
    CAN_FilterTypeDef can_filter;
    can_filter.FilterActivation = CAN_FILTER_ENABLE;    // 启用过滤器
    can_filter.FilterBank = 0;                          // 过滤器0
    can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0; // 分配到FIFO0
    can_filter.FilterIdHigh = 0x0000;                   // 标准ID
    can_filter.FilterIdLow = 0x0000;                    // 标准ID低16位
    can_filter.FilterMaskIdHigh = 0x0000;               // 掩码ID高16位
    can_filter.FilterMaskIdLow = 0x0000;                // 掩码ID低16位
    can_filter.FilterMode = CAN_FILTERMODE_IDMASK;      // 标识符掩码模式
    can_filter.FilterScale = CAN_FILTERSCALE_32BIT;     // 32位模式
    HAL_CAN_ConfigFilter(&hcan, &can_filter);

    // 开启 CAN通信
    HAL_CAN_Start(&hcan);
}

/**
 * @brief CAN 通信发送信息
 *
 * @param id
 * @param data 发送的数据
 * @param len 数据长度
 */
void Int_CAN_Send_Msg(uint32_t id, uint8_t *data, uint8_t len)
{
    // 等待发送邮箱有空闲
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
    {
    }

    // 填写数据帧信息
    CAN_TxHeaderTypeDef can_txheader;
    can_txheader.DLC = len;          // 指定将要传输的帧的长度
    can_txheader.ExtId = 0x00;       // 指定扩展标识符
    can_txheader.IDE = CAN_ID_STD;   // 指定将要传输的消息的标识符类型
    can_txheader.RTR = CAN_RTR_DATA; // 指定将要传输的消息的帧类型
    can_txheader.StdId = id;         // 指定标准标识符
    can_txheader.TransmitGlobalTime = DISABLE;
    uint32_t pTxMailbox; // 没有用到
    HAL_CAN_AddTxMessage(&hcan, &can_txheader, data, &pTxMailbox);
}

/**
 * @brief CAN 通信接收信息
 *
 * @param rxMsg 接收到的数据
 * @param msgCount 数据的个数
 */
void Int_CAN_Receive_Msg(RxDataMsg_Info *rxMsg, uint8_t *msgCount)
{
    // 获取接收到的信息的数量
    uint32_t rx_msg_num = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0);
    *msgCount = rx_msg_num;

    // 定义头结构体
    CAN_RxHeaderTypeDef can_rxheader;
    // 从FIFO0中读取消息
    for (uint32_t i = 0; i < rx_msg_num; i++)
    {
        HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rxheader, rxMsg[i].data);
        rxMsg[i].id = can_rxheader.StdId;
        rxMsg[i].len = can_rxheader.DLC;
    }
}
