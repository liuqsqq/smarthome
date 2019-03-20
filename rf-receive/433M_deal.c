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
** 函数功能： 定时器0 初始化 
** 入口参数： 无
** 出口参数： 无
** 返回值：无
** Note:无
--------------------------------------------------------------------------------------------------*/
void InitTimer0(void)
{
    TMOD = TMOD | 0x01;
    TH0 = T_RELOAD_HI;
	TL0 = T_RELOAD_LO;
    ET0 = 1;
} 
/*-------------------------------- ------------------------------------------------------------------
** 函数功能：定时器0 中断，
** 入口参数： 无
** 出口参数： 无
** 返回值：无
** Note:100us中断一次
--------------------------------------------------------------------------------------------------*/
void Int_Timer0() interrupt 1	  
{
	TH0 = T_RELOAD_HI;
	TL0 = T_RELOAD_LO;
}

//测试用，效果实在太差
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
					g_bRxState = 1;//接收到引导码,50ms低电平
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
					g_bRxState = 2;//接收到引导码,50ms低电平，25ms高电平
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
** 函数功能：外部中断0初始化 
** 入口参数： 无
** 出口参数： 无
** 返回值：无
** Note:无
--------------------------------------------------------------------------------------------------*/
void InitExt0(void)
{
	IT0 = 1;
	EX0 = 1;
} 
/*-------------------------------- ------------------------------------------------------------------
** 函数功能：外部中断0中断：解码函数，模仿红外解码
** 入口参数： 无
** 出口参数： 无
** 返回值：无
** Note:特别需要注意判断是否为1的那一句判断语句
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
					// 高电平大于1ms 小于2ms
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
** 函数功能：解码函数，模仿红外解码
** 入口参数： 无
** 出口参数： 无
** 返回值：无
** Note:特别需要注意判断是否为1的那一句判断语句,该函数应当在主函数当中循环调用
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

