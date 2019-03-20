//���Ժ����շ��Ĺ���   ����CCP���ܣ�ʹ����CCP0��Ϊ�������ţ�PCB˿ӡΪA4����ĸ�����Ӻ���ģ��� IR
//                     ���Ͳ���ģ��ʱ��ʹ��P55�ܽ���Ϊ���͹ܽţ����Ӻ���ģ���  IE 
//���õı���Э��Ϊ��NEC_upd6121 ��ͨң����ʹ�õ�Э��
//��Ƭ����Ƶ�ʣ�30MHZ    CCP����ʱ������Ϊ12��Ƶ��Ϊ2.5MHZ
//������ַ��WWW.JIXIN.PRO
#include "IRM3638T.h"
#include "uart.h"
#include "utils.h"
#include "stc8.h"
sbit IRM_E = P2^6;//����ⷢ��������ţ��ߵ�ƽ�������
sbit IRM_R = P1^7;//�����������ţ�����״̬�ߵ�ƽ�����ܵ�������ȡ���͵�ƽ

sbit rfsend = P3^3;

unsigned int Read_User_ID=0;//���յ���NECЭ����û���
unsigned char Read_User_Data=0;//���յ���NECЭ���������

static unsigned int  CCP_Bufer[70];//CCP���ջ����������ڽ��պ�������  ������+16λ�û���+8λ������+8λ���ݷ���+������ ����34�����ݣ���Ҫ68������
static unsigned char Read_F=0;//CCP�жϵ��ã�������յ�����������֡����ʼ����
static unsigned char CCP_Counter=0;//CCP���ջ�����������

void SendRfCode(unsigned int Code_User,unsigned char Code_Data);

//����һ�������壬����ת����׽�Ĵ������������ 0-65535
static union 
{
	unsigned int U16;
	unsigned char U8[2];
}High;//��ʾ������ߵ�ƽ�жϺʹ���͵�ƽ�жϵ���ʱ����
//���ս��룬ʹ��CCP0��
extern volatile unsigned int g_ulTimerCnt;

//��ʼ��CCP0  ʹ��CCP0�ܽţ����������źţ����ò����ܣ�����ߵ�ƽ��Ⱥ͵͵�ƽ���     ��������A4��ĸ
void Init_CCP0(void)
{
	//P_SW1 = 0X20;//CCP ȫ���ֲ���_3
	CF = 0;//PCA���������������־λ
	CR = 0;//PCA���������п���
	CCF2 = 0;//PCA2�жϱ�־λ
	CCF1 = 0;//PCA1�жϱ�־λ
	CCF0 = 0;//PCA0�жϱ�־λ
	CMOD = 0x00;//����PCAʱ��Դ1/12 & ��ֹPCA��ʱ������ж� ʵ��CCP����Ƶ���� 2.5MHZ             
	CCAPM0 = 0X71;//����CCP0�Ĳ���������ģʽ���������½��ض���������CCP0�ж�
	CL = 0;CH = 0;//��λPCA�Ĵ���
	CCAP0L = 0;CCAP0H = 0;//��λ��׽�Ĵ���
	CR = 1;//��PCA����
	Read_F = 0;//��ʼ��ʱ������
	CCP_Counter = 0;//��ʼ��ʱ��ֵΪ0
	rfsend = 1;
}
//PCA�ж���ڣ���������һ���ж���Ҫ�жϱ�־λ
void PCA_isr(void) interrupt 7
{
	//����ֻ����CCP0�ж�������������û���ж�����һ·CCP�������ж�
	CCF0 = 0;
	High.U8[0] = CCAP0H;
	High.U8[1] = CCAP0L;
	CCP_Bufer[CCP_Counter++] = High.U16;//������ʱ�䱣�浽��������
	//��Ƭ��ÿ����һ�Σ���ʾ0.4uS��ʱ��
	//���������ݴ���4mSС�� 5mS��������һ�����ݴ���8mSС��10mS����Ϊ��һ���ϸ��������
	if((CCP_Bufer[CCP_Counter-1]>10000)&&(CCP_Bufer[CCP_Counter-1]<12500)&&(CCP_Bufer[CCP_Counter-2]>20000)&&(CCP_Bufer[CCP_Counter-2]<25000))
	{
		CCP_Bufer[CCP_Counter-1] = 0;
		CCP_Bufer[CCP_Counter-2] = 0;
		CCP_Counter = 0;//�û����յ����ݷŵ��������ĵ�0λ
	}
	//����ȫ��Э����Ҫ66������������0-65��
	if(CCP_Counter >= 65)//
	{
		Read_F = 1;
		CCP_Counter = 69;
	}
	CL = 0;CH = 0;//��λPCA�Ĵ���
}
//����CCP�жϽ��յ����ݡ����������Ҫ�ŵ�����������ѭ�����òſ��ԡ�
//����������0��ʾ���յ�����Ч���ݣ�����1��ʾû�н��յ���Ч֡������2��ʾ�������
//ע�⣺���յ����ݣ��ǵ�λ��ǰ�ġ����յ��Ǹ��û��룬�ǵͰ�λ��ǰ��
unsigned char Get_NEC_Message(void)
{
	unsigned char i,j;
	unsigned char a[4];//���������4���ֽڵ����ݣ� �û���ͣ��û���ߣ������룬�����뷴��
	float b;
	if(Read_F == 1)
	{
		for(i=0;i<66;i++)//���������������ת��ΪmS��100������ΪЭ��������0.56mS��ЩС���������������ݷ���
		{
			b = CCP_Bufer[i] ;
			b /= 10;
			b *= 4;//����uS������ �൱�� X0.4 
			b /= 10;//mS��1 00��
			CCP_Bufer[i] = b;
			//UartSendByte(CCP_Bufer[i]);//���Դ��룬���ڴ��ڷ������յ�����
		}
		j=0; a[0]=0; a[1]=0; a[2]=0; a[3]=0;
		for(i=0;i<66;i+=2)//��ʼ�����ĸ��ֽ�����
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
		
////////////���Գ������ڴ��ڷ�������/////////////////////////////////////////////////////
		UartSendByte(a[0]);
		UartSendByte(a[1]);
		UartSendByte(a[2]);
		UartSendByte(a[3]);
//////////////////////////////////////////////////////////////////
		
		if((a[2]+a[3]) == 0XFF)// ������������ݷ�����ص���֤���������Ƿ���ȷ
		{
			Read_User_ID = a[1];
			Read_User_ID <<= 8;
			Read_User_ID += a[0];
			Read_User_Data = a[2];
			Read_F = 0;//�����־λ
			CCP_Counter = 0;//����������ж�����Ĵ���Ż�����ִ�У�����Ƭ�������һ֡���ݲ���Ӧ
			for(i=0;i<66;i++)//��ս�������
				CCP_Bufer[i] = 0;
			if (Read_User_ID == 0xbf00 && Read_User_Data == 0x43)
			{
				beep_control(2,1000);
				UartSendStr("decode success\n");
				//SendRfCode(0xcffc, 0x45);/* �˾����ڷ�����Ƶ�루433M������ʵ�ֺ���ת��Ƶ����  */
				rfsend = ~rfsend;/* ���յ������źź󣬿��ص� */
			}

			return 0;
		}
		else//������ִ���
		{
			Read_F = 0;//�����־λ
			CCP_Counter = 0;//����������ж�����Ĵ���Ż�����ִ�У�����Ƭ�������һ֡���ݲ���Ӧ
			for(i=0;i<66;i++)//��ս�������
			CCP_Bufer[i] = 0;
			Read_User_ID = 0;
			Read_User_Data =0;
			//UartSendStr("decode error\n");
			return 2;
		}
	}
	else
	{
		return 1;//��ʾû�н��յ���Ч֡
	}
}
//����Э�鷢�����ݣ�IO��ģ��ʱ��ʵ��
//��ʱ8.77us����һ�����ڵ�1/3 ��ʱ��  һ������26.3us,38KHZ
void Delay8_77us(void)//@30.000MHz
{
	volatile unsigned char i;
//	i = 59;
		i = 26;//stc8a8k  Y6
	while (--i);
}
//�ز�����
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
//�ز�������
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
//�������� 0 ������NECЭ��
void Send_NEC_0(void)
{
	Send_IRM(21);
	NO_Send_IRM(21);
}
//�������� 1 ������NECЭ��
void Send_NEC_1(void)
{
	Send_IRM(21);
	NO_Send_IRM(64);
}
//����һ֡������NEC��Ϣ ������+�û���Ͱ�λ+�û���߰�λ+8λ������+8λ������ķ���+�����롰0��
void Send_NEC_Message(unsigned int Code_User,unsigned char Code_Data)
{
	unsigned char i;
	unsigned int Code_User_2;
	Code_User_2 = Code_User;//ʹ����ʱ��������ֹ�޸��β�
	
	Send_IRM(342);
	NO_Send_IRM(171);//����������������һ��������
	
	for(i=0;i<16;i++)//����16λ���û���
	{
		if(Code_User_2 & 0X0001)
			Send_NEC_1();
		else
			Send_NEC_0();
		Code_User_2 >>= 1;
	}
	
	Code_User_2 = Code_Data;
	for(i=0;i<8;i++)//����8λ��������
	{
		if(Code_User_2 & 0X01)
			Send_NEC_1();
		else
			Send_NEC_0();
		Code_User_2 >>= 1;
	}
	
	Code_User_2 = (~Code_Data);
	for(i=0;i<8;i++)//����8λ��������ķ���
	{
		if(Code_User_2 & 0X01)
			Send_NEC_1();
		else
			Send_NEC_0();
		Code_User_2 >>= 1;
	}
	Send_NEC_0();//���ͽ�����
}


//����һ����Ƶ�룬ģ�º����źţ�������+�û���Ͱ�λ+�û���߰�λ+8λ������+8λ������ķ���+�����롰0��
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