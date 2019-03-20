#include "uart.h"
#include "stc8.h"

#define FOSC   (30000000UL)
#define BRT    (65536 - FOSC/115200/4)

//���ڳ�ʼ��
void UartInit()
{
	SCON = 0x50;
	T2L  =  BRT %256;
	T2H	 =  BRT /256;
	AUXR = 0x15;
}

//����һ���ֽ�
void UartSendByte(char dat)
{
	SBUF = dat;
	while(TI==0);
	TI = 0;
}

//�����ַ���
void UartSendStr(char* dats)
{
	while(*dats)
	{
		UartSendByte(*dats++);
	}
}
