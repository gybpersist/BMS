#ifndef __INT_CAN_H__
#define __INT_CAN_H__

#include "can.h"

// ����CAN��Ϣ�ṹ��
typedef struct
{
    uint32_t id;
    uint8_t data[8];
    uint8_t len;
} RxDataMsg_Info;

/**
 * @brief CAN ͨ�ų�ʼ��
 * 
 */
void Int_CAN_Init(void);

/**
 * @brief CAN ͨ�ŷ�����Ϣ
 * 
 * @param id 
 * @param data ���͵����� 
 * @param len ���ݳ���
 */
void Int_CAN_Send_Msg(uint32_t id, uint8_t *data, uint8_t len);

/**
 * @brief CAN ͨ�Ž�����Ϣ
 * 
 * @param rxMsg ���յ�������
 * @param msgCount ���ݵĸ���
 */
void Int_CAN_Receive_Msg(RxDataMsg_Info *rxMsg, uint8_t *msgCount);

#endif /* __INT_CAN_H__ */
