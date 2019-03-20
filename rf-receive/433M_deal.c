#include "433M_deal.h"
#include "STC89C52RC.h" 
#include "Extra_contral.h"
#include "uart.h"
#include "utils.h"

sbit g_pRfData = P3^2;

volatile u16  g_ulTimer0Cnt = 0;
volatile u8 g_bRxState = 0;

u8 g_ucReceiveCode[4] = {0};

/*-------------------------------- ------------------------------------------------------------------
** �������ܣ� ��ʱ��0 ��ʼ�� 
** ��ڲ����� ��
** ���ڲ����� ��
** ����ֵ����
** Note:��
--------------------------------------------------------------------------------------------------*/
void InitTimer0(void)
{
    TMOD = TMOD | 0x01;
    TH0 = T_RELOAD_HI;
	TL0 = T_RELOAD_LO;
    ET0 = 1;
} 
/*-------------------------------- ------------------------------------------------------------------
** �������ܣ���ʱ��0 �жϣ�
** ��ڲ����� ��
** ���ڲ����� ��
** ����ֵ����
** Note:100us�ж�һ��
--------------------------------------------------------------------------------------------------*/
void Int_Timer0() interrupt 1	  
{
	TH0 = T_RELOAD_HI;
	TL0 = T_RELOAD_LO;
}

//�����ã�Ч��ʵ��̫��
void Remote_Proess_Use_Timer()
{
	g_ulTimer0Cnt++;
	if (g_ulTimer0Cnt > 530)
	{
		g_ulTimer0Cnt = 0;
	}
	else
	{
		switch(g_bRxState)
		{
			case 0:
				if (1 == g_pRfData)
				{
					g_ulTimer0Cnt = 0;
					return;
				}				
				if (0 == g_pRfData && (g_ulTimer0Cnt >= 500 && g_ulTimer0Cnt <= 530))
				{
					g_bRxState = 1;//���յ�������,50ms�͵�ƽ
					g_ulTimer0Cnt = 0;   
				}	
				break;	
			#if 1
			case 1:
				if (0 == g_pRfData)
				{
					g_ulTimer0Cnt = 0;
					return;
				}
				if (1 == g_pRfData && (g_ulTimer0Cnt >= 180 && g_ulTimer0Cnt <= 190))
				{
					g_bRxState = 2;//���յ�������,50ms�͵�ƽ��25ms�ߵ�ƽ
					g_ulTimer0Cnt = 0;
				}	
				break;
			#endif
			case 2:
				break;
			default:
				break;
		}		
	}
}



/*-------------------------------- ------------------------------------------------------------------
** �������ܣ��ⲿ�ж�0��ʼ�� 
** ��ڲ����� ��
** ���ڲ����� ��
** ����ֵ����
** Note:��
--------------------------------------------------------------------------------------------------*/
void InitExt0(void)
{
	IT0 = 1;
	EX0 = 1;
} 
/*-------------------------------- ------------------------------------------------------------------
** �������ܣ��ⲿ�ж�0�жϣ����뺯����ģ�º������
** ��ڲ����� ��
** ���ڲ����� ��
** ����ֵ����
** Note:�ر���Ҫע���ж��Ƿ�Ϊ1����һ���ж����
--------------------------------------------------------------------------------------------------*/
void Int_Ext0() interrupt 0
{
	u32 i,j=0;
	u32 time_cnt = 0;
	u32 err_cnt = 0;
	DelayNus(10);
	if (g_pRfData == 0)
	{
		err_cnt = 400;
		while((g_pRfData==0) && (err_cnt > 0))
		{
			DelayNus(1);
			err_cnt--;
		}
		if (err_cnt == 0)
			return;
		if (g_pRfData == 1)
		{
			err_cnt = 200;
			while((g_pRfData == 1) && (err_cnt > 0))
			{
				DelayNus(1);
				err_cnt--;
			}
			if (err_cnt == 0)
				return;
			for (i = 0;i < 4;i++)
			{
				for(j = 0;j < 8;j++)
				{
					err_cnt = 35;
					while((g_pRfData == 0) && (err_cnt > 0))
					{
						DelayNus(1);
						err_cnt--;
					}
					if (err_cnt == 0)
						return;
					err_cnt = 50;
					time_cnt = 0;
					while((g_pRfData == 1) && (err_cnt > 0))
					{
						DelayNus(1);
						err_cnt--;
						time_cnt++;
					}
					if (err_cnt == 0)
						return;
					g_ucReceiveCode[i] >>= 1;
					// �ߵ�ƽ����1ms С��2ms
					if (time_cnt >= 40)
					{
						g_ucReceiveCode[i] |= 0x80;
					}
				}				
			}
			if ((g_ucReceiveCode[2] + g_ucReceiveCode[3]) == 0xff)
			{
				g_bRxState = 1;
			}
		}
	}
}

/*-------------------------------- ------------------------------------------------------------------
** �������ܣ����뺯����ģ�º������
** ��ڲ����� ��
** ���ڲ����� ��
** ����ֵ����
** Note:�ر���Ҫע���ж��Ƿ�Ϊ1����һ���ж����,�ú���Ӧ��������������ѭ������
--------------------------------------------------------------------------------------------------*/
u8 Remote_Process()
{
	u8 i,j,Count=0;
	Delay100us();
	if(g_pRfData == 0)
	{  
		for(Count=0; Count < 100 ; Count++);
		{
			Delay100us();//100us*100=10ms
			if(g_pRfData == 1)
			{
				return -1;
			}
		}
		while(g_pRfData == 0);

		for(Count=0; Count <  50; Count++);
		{
			Delay100us();//100us*50=5ms
			if(g_pRfData == 0)
			{
				return -1;
			}
		}
		while(g_pRfData == 1);
		
		for(j = 0;j < 4;j++)
		{
			for(i = 0;i < 8;i++)
			{
				Count=0;
				do
				{
					Delay100us();
					Count++;
					if(Count > 15)
					{
						return -1;
					}
				}while(g_pRfData == 0);
				Count=0;
				do
				{ 
					Delay100us();
					Count++;
					if(Count > 25)
					{
						return -1;
					}
				}while(g_pRfData == 1);
				g_ucReceiveCode[j] >>= 1;
				if(Count > 11)
				{
					g_ucReceiveCode[j] |= 0x80;
				}
			}
		}
		if ((g_ucReceiveCode[2] + g_ucReceiveCode[3]) == 0xff)
		{
			return 1;
		}
	}	
	return 0;
}

