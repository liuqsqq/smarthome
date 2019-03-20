#include "Extra_contral.h"
#include "STC89C52RC.h" 
#include "Uart.h"
#include "433M_deal.h"
#include "utils.h"

extern volatile u8 g_bRxState;
extern u8 g_ucReceiveCode[4];



sbit dcs_control = P0^0;


/*-------------------------------- ------------------------------------------------------------------
** �������ܣ�ϵͳ��ʼ�� 
** ��ڲ����� ��
** ���ڲ����� ��
** ����ֵ����
** Note:��
--------------------------------------------------------------------------------------------------*/
void InitSystem(void)
{
    InitRs232();/* ��ʼ������ */
    //InitTimer0();/* ��ʼ����ʱ��0*/
    EA  = 1;     /* ��Ƭ�����ж����� */    
} 
/*-------------------------------- ------------------------------------------------------------------
** �������ܣ�������
** ��ڲ�������
** ���ڲ����� ��
** ����ֵ����
** Note:��
--------------------------------------------------------------------------------------------------*/
void main(void)
{
	u8 i = 0;
	u8 result = -1;
    InitSystem();
    DelayNms(100);
	DelayNus(1);
	Delay100us();
    UartSendStr("System is Ready!\r\n");   
	InitExt0();	
	g_bRxState = 0;
	dcs_control = 1;
  	while(1)
    {
		while(g_bRxState == 0);
		EX0 = 0;
		if ((g_ucReceiveCode[0] == 0xfc) && (g_ucReceiveCode[1] == 0xcf) 
			&& (g_ucReceiveCode[2] == 0x45) )
		{
			dcs_control = ~dcs_control;
		}
		for (i = 0;i < 4;i++)
		{
			UartSendByte(g_ucReceiveCode[i]);	
			g_ucReceiveCode[i] = 0x00;
		}
		g_bRxState = 0;
		EX0 = 1;
    }
}