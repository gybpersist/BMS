#ifndef __INT_CAN_H__
#define __INT_CAN_H__

#include "can.h"

// 定义CAN消息结构体
typedef struct
{
    uint32_t id;
    uint8_t data[8];
    uint8_t len;
} RxDataMsg_Info;

/**
 * @brief CAN 通信初始化
 * 
 */
void Int_CAN_Init(void);

/**
 * @brief CAN 通信发送信息
 * 
 * @param id 
 * @param data 发送的数据 
 * @param len 数据长度
 */
void Int_CAN_Send_Msg(uint32_t id, uint8_t *data, uint8_t len);

/**
 * @brief CAN 通信接收信息
 * 
 * @param rxMsg 接收到的数据
 * @param msgCount 数据的个数
 */
void Int_CAN_Receive_Msg(RxDataMsg_Info *rxMsg, uint8_t *msgCount);

#endif /* __INT_CAN_H__ */
