#include "Com_debug.h"

// HAL �ض��� fputc ���������� printf ����
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000); // ʹ�� HAL_UART_Transmit ���������ַ�
  return ch;                                           // ���ط��͵��ַ�
}
