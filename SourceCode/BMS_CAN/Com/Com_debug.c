#include "Com_debug.h"

// HAL 重定义 fputc 函数，用于 printf 函数
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000); // 使用 HAL_UART_Transmit 函数发送字符
  return ch;                                           // 返回发送的字符
}
