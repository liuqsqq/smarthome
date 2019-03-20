sbit  OP=P3^4; //红外发射管的亮灭控制位

static unsigned int count;        //延时计数器
static unsigned int endcount;     //终止延时计数
static unsigned char flag;        //红外发射标志

char iraddr1; //十六位地址低位地址
char iraddr2; //十六位地址高位地址
void SendIRdata(char p_irdata);//发送子函数
void delay();

void Init_IRM()
{
	count = 0;
    flag = 0;        //无载波
    OP = 0;          //不亮
    
	EA = 1;          //允许总中断
    TMOD = 0x11;     //定时器模式1 
    
	ET0 = 1;         //允许定时器0中断
    TH0 = 0xFF;  
    TL0 = 0xE6;     //T0产生38k方波，用作红外载波，也就是26us
    TR0 = 1;        //定时器0开始计时
    
	iraddr1=3;       
    iraddr2=252;    //发送地址为1111 1100 0000 0011
}

//定时器0中断处理函数 
void Time0Isr(void) interrupt 1 
{ 
    TH0=0xFF; 
    TL0=0xE6;         //38K
    count++; 
     
    if (flag==1)//允许发射38K红外载波
    {
        OP=~OP;
    }
    else
    {
        OP = 0;  //io口设置为低电平
    }
} 

/红外信号发射函数
void SendIRdata(char p_irdata)
{ 
    int i;
    char irdata=p_irdata; 
     
    /***************************************************************/
     
    //前导码9ms高电平 
    //346个中断间隔,9000us/26us = 346
    endcount = 346;
    flag=1;//开始发射红外信号
    count=0;
    while(count<endcount);//发射完毕
 
 
    //发射4.5ms低电平
    //173个中断间隔,4500us/26us = 173
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