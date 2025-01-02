/*
 * rgb.c
 *
 * Created: 1/2/2025 12:51:41 PM
 *  Author: itsLydt
 */ 

#include "rgb.h"

const struct LEDColor_t OFF = {};

void LED_Init(){
	uint32_t pin_mask = (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);
	GPIO_SetPortDirection(LED_PORT, pin_mask, GPIO_OUT);
	LED_SetColor(OFF);
}

void LED_SetColor(struct LEDColor_t color){

}