#ifndef _SERIALLCD_H
#define _SERIALLCD_H


#include "main.h"


#ifdef __cplusplus
extern "C" {
#endif


enum object
{
	Coca_cola_Can_330ml = 0,
	Fanta_Can_330ml,
	Master_Kong_lced_Black_Tea_500ml,
	Nongfu_Spring_550ml,
	Pepsi_Can_330ml,
	Scream_550ml,
	Sprite_Can_330ml,
	WALOVI_Can_310ml
};


int SerialLCD_Init(const char *device, int baudrate);
int SerialLCD_Close(void);
int SerialLCD_Thread_Start(PT_Manager ptManager);
int SerialLCD_Thread_Join(void);


#ifdef __cplusplus
}
#endif


#endif
