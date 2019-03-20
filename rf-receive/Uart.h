#ifndef _UART_H_
#define _UART_H_

#include "Extra_contral.h"

void InitRs232(void);
void UartSendByte(u8 ch);
void UartSendStr(u8* pstr);

#endif