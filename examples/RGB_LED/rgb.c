/*
 * rgb.c
 *
 * Created: 1/2/2025 12:51:41 PM
 *  Author: itsLydt
 */ 

#include "rgb.h"

const struct LEDColor_t OFF = {};
const struct LEDColor_t RED = {.r=1};
const struct LEDColor_t GREEN = {.g=1};
const struct LEDColor_t BLUE = {.b=1};
const struct LEDColor_t YELLOW = {.r=1,.g=1};
const struct LEDColor_t VIOLET = {.r=1,.b=1};
const struct LEDColor_t TEAL = {.g=1,.b=1};
const struct LEDColor_t WHITE = {.r=1,.b=1,.g=1};

void LED_Init(){
	uint32_t pin_mask = (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);
	GPIO_SetPortDirection(LED_PORT, pin_mask, GPIO_OUT);
	LED_SetColor(OFF);
}

void LED_SetColor(struct LEDColor_t color){

}