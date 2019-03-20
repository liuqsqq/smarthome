#include "utils.h"
#include "stc8.h"
#include "uart.h"

sbit beep = P5^5;/* 板载蜂鸣器 */
sbit led  = P5^4;/* 板载led灯 */

#define FOSC   (30000000UL) /* 晶振为30M */


//延时1ms
void Delay1ms(unsigned int times)
{
	unsigned int  i = 0;
	unsigned int j = 0;
	for(i = 0;i <  times;i++){
		for(j = 0;j < 488;j++);
	}
}

//蜂鸣器控制，times为几次，dura为间隔时间
void beep_control(int times, int dura)
{
	int i = 0;
	int j = 0;
	int k = 0;
	for(j = times;j > 0;j--)
	{
		Delay1ms(500);
		for (k = dura; k > 0;k--)
		{
			for(i = 0;i < 128;i++);
			beep = ~beep;			
		}
	}
	beep = 0;
}
void led_on()
{
	led = 0;
}

void led_off()
{
	led = 1;
}

//led灯闪烁
void led_blink(int times)
{
	int i = 0;
	for(i = times;i > 0;i--)
	{
		led_on();
		Delay1ms(300);
		led_off();
		Delay1ms(300);
	}
}

//初始化GPIO口，P5^4\P5^5 推挽输出，P3^3\P3^4推挽输出
void init_gpio()
{
	P5M1 = 0x00;
	P5M0 = 0x30;
	P3M1 = 0x00;
	P3M0 = 0x18;
	beep = 0;
}

//初始化定时器0，模式0、10ms定时、开定时器0中断
void init_timer0()
{
	TMOD = 0x00;
	TH0  = (65536 -  10*FOSC/12/1000)/256;//10ms
	TL0  = (65536 -  10*FOSC/12/1000)%256;
	TR0 = 1;
	ET0 = 1;
}

//初始化外部中断0、上升沿和下降沿触发、开外部0中断
void init_ext0()
{
	IT0 = 0;
	EX0 = 1;
}
