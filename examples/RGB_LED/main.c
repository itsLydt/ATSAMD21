/*
 * main.c
 *
 * Created: 1/2/2025 11:34:59 AM
 * Author : itsLydt
 */ 


#include "sam.h"

#include "config_led.h"
#include "rgb.h"

int main(void)
{
	int counter = 100000;
    LED_Init();
	
    while (1) 
    {
		for(int i = 0; i < counter; ++i);
		LED_SetColor(RED);
		
		for(int i = 0; i < counter; ++i);
		LED_SetColor(GREEN);
		
		for(int i = 0; i < counter; ++i);
		LED_SetColor(BLUE);
    }
}
