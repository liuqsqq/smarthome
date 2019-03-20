sbit  OP=P3^4; //���ⷢ��ܵ��������λ

static unsigned int count;        //��ʱ������
static unsigned int endcount;     //��ֹ��ʱ����
static unsigned char flag;        //���ⷢ���־

char iraddr1; //ʮ��λ��ַ��λ��ַ
char iraddr2; //ʮ��λ��ַ��λ��ַ
void SendIRdata(char p_irdata);//�����Ӻ���
void delay();

void Init_IRM()
{
	count = 0;
    flag = 0;        //���ز�
    OP = 0;          //����
    
	EA = 1;          //�������ж�
    TMOD = 0x11;     //��ʱ��ģʽ1 
    
	ET0 = 1;         //����ʱ��0�ж�
    TH0 = 0xFF;  
    TL0 = 0xE6;     //T0����38k���������������ز���Ҳ����26us
    TR0 = 1;        //��ʱ��0��ʼ��ʱ
    
	iraddr1=3;       
    iraddr2=252;    //���͵�ַΪ1111 1100 0000 0011
}

//��ʱ��0�жϴ����� 
void Time0Isr(void) interrupt 1 
{ 
    TH0=0xFF; 
    TL0=0xE6;         //38K
    count++; 
     
    if (flag==1)//������38K�����ز�
    {
        OP=~OP;
    }
    else
    {
        OP = 0;  //io������Ϊ�͵�ƽ
    }
} 

/�����źŷ��亯��
void SendIRdata(char p_irdata)
{ 
    int i;
    char irdata=p_irdata; 
     
    /***************************************************************/
     
    //ǰ����9ms�ߵ�ƽ 
    //346���жϼ��,9000us/26us = 346
    endcount = 346;
    flag=1;//��ʼ��������ź�
    count=0;
    while(count<endcount);//�������
 
 
    //����4.5ms�͵�ƽ
    //173���жϼ��,4500us/26us = 173
    endcount = 173;
    flag=0;
    count=0;
    while(count<endcount); 
 
    /***************************************************************/
    irdata=iraddr1;
    for(i=0;i<8;i++)
    {
		endcount = 21;//560us/26us = 21
        flag=1;
        count=0;
        while(count<endcount);
        if(irdata-(irdata/2)*2)
        {
            endcount=64; //endcount = 64, 64*26us = 1680us
		}
          else         
        {
            endcount=21;//21*26us = 560us;
        }
        flag=0;
        count=0;
        while(count<endcount);
        irdata=irdata>>1;
    }
 
    irdata=iraddr2;
    for(i=0;i<8;i++)
    {
        endcount = 21;
        flag=1;
        count=0;
        while(count<endcount); 
        if(irdata-(irdata/2)*2)
        {
            endcount = 64;
 
        }
        else
        {
            endcount = 21;
        }
 
        flag=0;
        count=0;
        while(count<endcount); 
        irdata=irdata>>1;
    }/***************************************************************/
     
    irdata=p_irdata;
     
    for(i=0;i<8;i++)
    {
        endcount = 21;
        flag=1;
        count=0;        
        while(count<endcount); 
        if(irdata-(irdata/2)*2)
        { 
            endcount=64;; 
        }
        else
        {
            endcount = 21; 
        }
        flag=0;
        count=0;
        while(count<endcount);
        irdata=irdata>>1;       
    }
    irdata=~p_irdata;
    for(i=0;i<8;i++)
    {
        endcount = 21;
        flag=1;
        count=0;
        while(count<endcount); 
        if(irdata-(irdata/2)*2)
        {
            endcount = 64;
        }
        else
        {
            endcount = 21;
        }
        flag=0;
        count=0;
        while(count<endcount); 
        irdata=irdata>>1;
    }
}