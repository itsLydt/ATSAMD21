/*
 * config_led.h
 *
 * Created: 1/2/2025 12:41:52 PM
 *  Author: itsLydt
 */ 


#ifndef CONFIG_LED_H_
#define CONFIG_LED_H_

#include "gpio.h"

/* port and pin assignments for RGB LED */
PortGroup* const LED_PORT = GPIOB;
const uint8_t LED_RED = 4;
const uint8_t LED_GREEN = 5;
const uint8_t LED_BLUE = 6;


#endif /* CONFIG_LED_H_ */