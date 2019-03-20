#ifndef _EXTRA_CONTROL_H
#define _EXTRA_CONTROL_H


typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned int u32;
typedef signed int s32;
typedef enum {false = 0, true = !false} bool;

#define MAX_BUFFER_SIZE	16

#define INTERVAL_MS(Nms)  ((Nms/10) ? (Nms/10):(1))

#define FOSC 11059200L      //System frequency
#define BAUD 9600           //UART baudrate

/* 11.0592¾§Õñ50US¼ÆÊý³õÖµ*/
#define T_RELOAD   (65536 - 100*FOSC/12/1000/1000)
#define T_RELOAD_LO (T_RELOAD & 0x00ff)
#define T_RELOAD_HI (T_RELOAD >> 8)


#define DEBUG_FLAG 0
#define DEBUG_FLAG_TIMER0 0

#endif
