//测试红外收发的功能   打开了CCP功能，使用了CCP0作为接收引脚，PCB丝印为A4的排母，连接红外模块的 IR
//                     发送采用模拟时序，使用P55管脚作为发送管脚，连接红外模块的  IE 
//采用的编码协议为：NEC_upd6121 普通遥控器使用的协议
//单片机的频率：30MHZ    CCP运行时钟设置为12分频，为2.5MHZ
//作者网址：WWW.JIXIN.PRO
#include "IRM3638T.h"
#include "uart.h"
#include "utils.h"
#include "stc8.h"
sbit IRM_E = P2^6;//红外光发射控制引脚，高电平发射红外
sbit IRM_R = P1^7;//红外光接收引脚，正常状态高电平，接受到红外光读取到低电平

sbit rfsend = P3^3;

unsigned int Read_User_ID=0;//接收到的NEC协议的用户码
unsigned char Read_User_Data=0;//接收到的NEC协议的数据码

static unsigned int  CCP_Bufer[70];//CCP接收缓冲区，用于接收红外数据  引导码+16位用户码+8位数据码+8位数据反码+结束码 ，共34个数据，需要68个脉宽
static unsigned char Read_F=0;//CCP中断调用，如果接收到了完整数据帧，开始解码
static unsigned char CCP_Counter=0;//CCP接收缓冲区计数用

void SendRfCode(unsigned int Code_User,unsigned char Code_Data);

//定义一个联合体，用于转换捕捉寄存器里面的数据 0-65535
static union 
{
	unsigned int U16;
	unsigned char U8[2];
}High;//表示，处理高电平中断和处理低电平中断的临时变量
//接收解码，使用CCP0。
extern volatile unsigned int g_ulTimerCnt;

//初始化CCP0  使用CCP0管脚，采样脉冲信号，利用捕获功能，计算高电平宽度和低电平宽度     板子上面A4排母
void Init_CCP0(void)
{
	//P_SW1 = 0X20;//CCP 全部分布到_3
	CF = 0;//PCA计数器阵列溢出标志位
	CR = 0;//PCA计数器阵列开关
	CCF2 = 0;//PCA2中断标志位
	CCF1 = 0;//PCA1中断标志位
	CCF0 = 0;//PCA0中断标志位
	CMOD = 0x00;//设置PCA时钟源1/12 & 禁止PCA定时器溢出中断 实际CCP运行频率是 2.5MHZ             
	CCAPM0 = 0X71;//设置CCP0的参数，捕获模式，上升沿下降沿都捕获，允许CCP0中断
	CL = 0;CH = 0;//复位PCA寄存器
	CCAP0L = 0;CCAP0H = 0;//复位捕捉寄存器
	CR = 1;//打开PCA阵列
	Read_F = 0;//初始化时候清零
	CCP_Counter = 0;//初始化时候赋值为0
	rfsend = 1;
}
//PCA中断入口，具体是哪一个中断需要判断标志位
void PCA_isr(void) interrupt 7
{
	//由于只打开了CCP0中断允许，所以这里没有判断是哪一路CCP产生的中断
	CCF0 = 0;
	High.U8[0] = CCAP0H;
	High.U8[1] = CCAP0L;
	CCP_Bufer[CCP_Counter++] = High.U16;//把脉宽时间保存到数组里面
	//单片机每计数一次，表示0.4uS的时间
	//如果这个数据大于4mS小于 5mS，而且上一个数据大于8mS小于10mS，认为是一个合格的引导码
	if((CCP_Bufer[CCP_Counter-1]>10000)&&(CCP_Bufer[CCP_Counter-1]<12500)&&(CCP_Bufer[CCP_Counter-2]>20000)&&(CCP_Bufer[CCP_Counter-2]<25000))
	{
		CCP_Bufer[CCP_Counter-1] = 0;
		CCP_Bufer[CCP_Counter-2] = 0;
		CCP_Counter = 0;//用户接收的数据放到缓冲区的第0位
	}
	//接收全部协议需要66个脉宽，所以是0-65，
	if(CCP_Counter >= 65)//
	{
		Read_F = 1;
		CCP_Counter = 69;
	}
	CL = 0;CH = 0;//复位PCA寄存器
}
//解码CCP中断接收的数据。这个函数需要放到主函数里面循环调用才可以。
//本函数返回0表示接收到了有效数据，返回1表示没有接收到有效帧，返回2表示解码错误
//注意：接收的数据，是低位在前的。接收的那个用户码，是低八位在前。
unsigned char Get_NEC_Message(void)
{
	unsigned char i,j;
	unsigned char a[4];//将会解析出4个字节的数据， 用户码低，用户码高，数据码，数据码反码
	float b;
	if(Read_F == 1)
	{
		for(i=0;i<66;i++)//把数组里面的脉宽转换为mS的100倍。因为协议里面有0.56mS这些小数，这样便于数据分析
		{
			b = CCP_Bufer[i] ;
			b /= 10;
			b *= 4;//计算uS的数据 相当于 X0.4 
			b /= 10;//mS的1 00倍
			CCP_Bufer[i] = b;
			//UartSendByte(CCP_Bufer[i]);//测试代码，用于串口分析接收的脉宽
		}
		j=0; a[0]=0; a[1]=0; a[2]=0; a[3]=0;
		for(i=0;i<66;i+=2)//开始解码四个字节数据
		{
			if(j < 8)
			{
				a[0] >>= 1;
				if( (CCP_Bufer[i+1]>CCP_Bufer[i]) && ((CCP_Bufer[i+1]-CCP_Bufer[i])>56) )//
					a[0] |= 0X80;
			}
			if((j > 7) && (j < 16))
			{
				a[1] >>= 1;
				if( (CCP_Bufer[i+1]>CCP_Bufer[i]) && ((CCP_Bufer[i+1]-CCP_Bufer[i])>56) )//
					a[1] |= 0X80;
			}
			if((j > 15) && (j < 24))
			{
				a[2] >>= 1;
				if( (CCP_Bufer[i+1]>CCP_Bufer[i]) && ((CCP_Bufer[i+1]-CCP_Bufer[i])>56) )//
					a[2] |= 0X80;
			}
			if((j > 23) && (j < 32))
			{
				a[3] >>= 1;
				if( (CCP_Bufer[i+1]>CCP_Bufer[i]) && ((CCP_Bufer[i+1]-CCP_Bufer[i])>56) )//
					a[3] |= 0X80;
			}
			j++;
		}
		
////////////测试程序，用于串口分析数据/////////////////////////////////////////////////////
		UartSendByte(a[0]);
		UartSendByte(a[1]);
		UartSendByte(a[2]);
		UartSendByte(a[3]);
//////////////////////////////////////////////////////////////////
		
		if((a[2]+a[3]) == 0XFF)// 用数据码和数据反码的特点验证接收数据是否正确
		{
			Read_User_ID = a[1];
			Read_User_ID <<= 8;
			Read_User_ID += a[0];
			Read_User_Data = a[2];
			Read_F = 0;//清零标志位
			CCP_Counter = 0;//这里清零后，中断里面的处理才会正常执行，否则单片机会对下一帧数据不响应
			for(i=0;i<66;i++)//清空接收数组
				CCP_Bufer[i] = 0;
			if (Read_User_ID == 0xbf00 && Read_User_Data == 0x43)
			{
				beep_control(2,1000);
				UartSendStr("decode success\n");
				//SendRfCode(0xcffc, 0x45);/* 此句用于发送射频码（433M），即实现红外转射频功能  */
				rfsend = ~rfsend;/* 接收到红外信号后，开关灯 */
			}

			return 0;
		}
		else//解码出现错误
		{
			Read_F = 0;//清零标志位
			CCP_Counter = 0;//这里清零后，中断里面的处理才会正常执行，否则单片机会对下一帧数据不响应
			for(i=0;i<66;i++)//清空接收数组
			CCP_Bufer[i] = 0;
			Read_User_ID = 0;
			Read_User_Data =0;
			//UartSendStr("decode error\n");
			return 2;
		}
	}
	else
	{
		return 1;//表示没有接收到有效帧
	}
}
//根据协议发送数据，IO口模拟时序实现
//延时8.77us，是一个周期的1/3 的时间  一个周期26.3us,38KHZ
void Delay8_77us(void)//@30.000MHz
{
	volatile unsigned char i;
//	i = 59;
		i = 26;//stc8a8k  Y6
	while (--i);
}
//载波发射
void Send_IRM(volatile unsigned int i)
{
	while(i--)
	{
		IRM_E = 1;
		Delay8_77us();
		IRM_E = 0;
		Delay8_77us();
		Delay8_77us();
	}
}
//载波不发射
void NO_Send_IRM(volatile unsigned int i)
{
	while(i--)
	{
		IRM_E = 0;
		Delay8_77us();
		Delay8_77us();
		Delay8_77us();
	}
}
//发射数据 0 ，符合NEC协议
void Send_NEC_0(void)
{
	Send_IRM(21);
	NO_Send_IRM(21);
}
//发射数据 1 ，符合NEC协议
void Send_NEC_1(void)
{
	Send_IRM(21);
	NO_Send_IRM(64);
}
//发射一帧完整的NEC信息 引导码+用户码低八位+用户码高八位+8位数据码+8位数据码的反码+结束码“0”
void Send_NEC_Message(unsigned int Code_User,unsigned char Code_Data)
{
	unsigned char i;
	unsigned int Code_User_2;
	Code_User_2 = Code_User;//使用临时变量，防止修改形参
	
	Send_IRM(342);
	NO_Send_IRM(171);//这两个函数构成了一个引导码
	
	for(i=0;i<16;i++)//发送16位的用户码
	{
		if(Code_User_2 & 0X0001)
			Send_NEC_1();
		else
			Send_NEC_0();
		Code_User_2 >>= 1;
	}
	
	Code_User_2 = Code_Data;
	for(i=0;i<8;i++)//发送8位的数据码
	{
		if(Code_User_2 & 0X01)
			Send_NEC_1();
		else
			Send_NEC_0();
		Code_User_2 >>= 1;
	}
	
	Code_User_2 = (~Code_Data);
	for(i=0;i<8;i++)//发送8位的数据码的反码
	{
		if(Code_User_2 & 0X01)
			Send_NEC_1();
		else
			Send_NEC_0();
		Code_User_2 >>= 1;
	}
	Send_NEC_0();//发送结束码
}


//发射一组射频码，模仿红外信号，引导码+用户码低八位+用户码高八位+8位数据码+8位数据码的反码+结束码“0”
void SendRfCode(unsigned int Code_User,unsigned char Code_Data)
{
	unsigned int i = 0;
	unsigned int Code_User_tmp = 0;
	rfsend = 1;
	g_ulTimerCnt = 0;
	TR0 = 1;
	while(g_ulTimerCnt < 20);
	TR0 = 0;
	g_ulTimerCnt = 0;
	rfsend = 0;
	TR0 = 1;
	while(g_ulTimerCnt < 200);
	TR0 = 0;
	g_ulTimerCnt = 0;
	rfsend = 1;
	TR0 = 1;
	while(g_ulTimerCnt < 100);
	TR0 = 0;
	Code_User_tmp = Code_User;
	for (i = 0; i < 16;i++)
	{
		if(Code_User_tmp & 0x0001)
		{
			g_ulTimerCnt = 0;
			rfsend = 0;
			TR0 = 1;
			while(g_ulTimerCnt <= 20);
			TR0 = 0;
			g_ulTimerCnt = 0;
			rfsend = 1;
		    TR0 = 1;
			while(g_ulTimerCnt <= 40);
		    TR0 = 0;
		}
		else
		{
			g_ulTimerCnt = 0;
			rfsend = 0;
			TR0 = 1;
			while(g_ulTimerCnt <= 20);
			TR0 = 0;
			g_ulTimerCnt = 0;
			rfsend = 1;
		    TR0 = 1;
			while(g_ulTimerCnt <= 20);
		    TR0 = 0;
		}
		Code_User_tmp >>= 1;	
	}
	Code_User_tmp = Code_Data;
	for (i = 0;i < 8;i++)
	{
		if(Code_User_tmp & 0x01)
		{
			g_ulTimerCnt = 0;
			rfsend = 0;
			TR0 = 1;
			while(g_ulTimerCnt <= 20);
			TR0 = 0;
			g_ulTimerCnt = 0;
			rfsend = 1;
		    TR0 = 1;
			while(g_ulTimerCnt <= 40);
		    TR0 = 0;		
		}
		else
		{
			g_ulTimerCnt = 0;
			rfsend = 0;
			TR0 = 1;
			while(g_ulTimerCnt <= 20);
			TR0 = 0;
			g_ulTimerCnt = 0;
			rfsend = 1;
		    TR0 = 1;
			while(g_ulTimerCnt <= 20);
		    TR0 = 0;
		}
		Code_User_tmp >>= 1;
	}
	Code_User_tmp = ~Code_Data;
	for (i = 0;i < 8;i++)
	{
		if(Code_User_tmp & 0x01)
		{
			g_ulTimerCnt = 0;
			rfsend = 0;
			TR0 = 1;
			while(g_ulTimerCnt <= 20);
			TR0 = 0;
			g_ulTimerCnt = 0;
			rfsend = 1;
		    TR0 = 1;
			while(g_ulTimerCnt <= 40);
		    TR0 = 0;			
		}
		else
		{
			g_ulTimerCnt = 0;
			rfsend = 0;
			TR0 = 1;
			while(g_ulTimerCnt <= 20);
			TR0 = 0;
			g_ulTimerCnt = 0;
			rfsend = 1;
		    TR0 = 1;
			while(g_ulTimerCnt <= 20);
		    TR0 = 0;
		}
		Code_User_tmp >>= 1;
	}	
	g_ulTimerCnt = 0;
	TR0 = 0;
	rfsend = 0;
}
//