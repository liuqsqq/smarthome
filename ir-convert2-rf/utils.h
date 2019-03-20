#ifndef _UTILS_H_
#define _UTILS_H_

void Delay1ms(unsigned int times);
void beep_control(int times, int dura);
void init_gpio();
void led_blink(int times);
void led_off();
void led_on();

void init_timer0();
void init_ext0();
typedef enum {false = 0, true = !false} bool;

#endif