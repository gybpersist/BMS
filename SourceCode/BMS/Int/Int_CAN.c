#include "Int_CAN.h"

/**
 * @brief CAN ͨ�ų�ʼ��
 *
 */
void Int_CAN_Init(void)
{
    // ���ù�����
    CAN_FilterTypeDef can_filter;
    can_filter.FilterActivation = CAN_FILTER_ENABLE;    // ���ù�����
    can_filter.FilterBank = 0;                          // ������0
    can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0; // ���䵽FIFO0
    can_filter.FilterIdHigh = 0x0000;                   // ��׼ID
    can_filter.FilterIdLow = 0x0000;                    // ��׼ID��16λ
    can_filter.FilterMaskIdHigh = 0x0000;               // ����ID��16λ
    can_filter.FilterMaskIdLow = 0x0000;                // ����ID��16λ
    can_filter.FilterMode = CAN_FILTERMODE_IDMASK;      // ��ʶ������ģʽ
    can_filter.FilterScale = CAN_FILTERSCALE_32BIT;     // 32λģʽ
    HAL_CAN_ConfigFilter(&hcan, &can_filter);

    // ���� CANͨ��
    HAL_CAN_Start(&hcan);
}

/**
 * @brief CAN ͨ�ŷ�����Ϣ
 *
 * @param id
 * @param data ���͵�����
 * @param len ���ݳ���
 */
void Int_CAN_Send_Msg(uint32_t id, uint8_t *data, uint8_t len)
{
    // �ȴ����������п���
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
    {
    }

    // ��д����֡��Ϣ
    CAN_TxHeaderTypeDef can_txheader;
    can_txheader.DLC = len;          // ָ����Ҫ�����֡�ĳ���
    can_txheader.ExtId = 0x00;       // ָ����չ��ʶ��
    can_txheader.IDE = CAN_ID_STD;   // ָ����Ҫ�������Ϣ�ı�ʶ������
    can_txheader.RTR = CAN_RTR_DATA; // ָ����Ҫ�������Ϣ��֡����
    can_txheader.StdId = id;         // ָ����׼��ʶ��
    can_txheader.TransmitGlobalTime = DISABLE;
    uint32_t pTxMailbox; // û���õ�
    HAL_CAN_AddTxMessage(&hcan, &can_txheader, data, &pTxMailbox);
}

/**
 * @brief CAN ͨ�Ž�����Ϣ
 *
 * @param rxMsg ���յ�������
 * @param msgCount ���ݵĸ���
 */
void Int_CAN_Receive_Msg(RxDataMsg_Info *rxMsg, uint8_t *msgCount)
{
    // ��ȡ���յ�����Ϣ������
    uint32_t rx_msg_num = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0);
    *msgCount = rx_msg_num;

    // ����ͷ�ṹ��
    CAN_RxHeaderTypeDef can_rxheader;
    // ��FIFO0�ж�ȡ��Ϣ
    for (uint32_t i = 0; i < rx_msg_num; i++)
    {
        HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rxheader, rxMsg[i].data);
        rxMsg[i].id = can_rxheader.StdId;
        rxMsg[i].len = can_rxheader.DLC;
    }
}
