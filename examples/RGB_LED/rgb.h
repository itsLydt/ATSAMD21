/*
 * rgb.h
 *
 * Created: 1/2/2025 12:17:07 PM
 *  Author: itsLydt
 */ 


#ifndef RGB_H_
#define RGB_H_

#include "gpio.h"

extern PortGroup* const LED_PORT;
extern const uint8_t LED_RED;
extern const uint8_t LED_GREEN;
extern const uint8_t LED_BLUE;

struct LEDColor_t {
	uint8_t r	:1;
	uint8_t g	:1;
	uint8_t b	:1;
	uint8_t		:5;
};

extern const struct LEDColor_t RED;
extern const struct LEDColor_t GREEN;
extern const struct LEDColor_t BLUE;
extern const struct LEDColor_t YELLOW;
extern const struct LEDColor_t VIOLET;
extern const struct LEDColor_t TEAL;
extern const struct LEDColor_t WHITE;
extern const struct LEDColor_t OFF;

/* set up the pins for the RGB LED and turn them off */
void LED_Init();

/* set RGB LED to specified color */
void LED_SetColor(struct LEDColor_t color);


#endif /* RGB_H_ */