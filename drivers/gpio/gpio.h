/*
 * gpio.h
 *
 * Created: 6/24/2024 5:03:04 PM
 *  Author: itsLydt
 */ 


#ifndef GPIO_H_
#define GPIO_H_

#include <stdbool.h>
#include <stdint.h>

#include "sam.h"

//TODO: IOBUS implementation?
#define GPIO_BASE_ADDR 0x41004400
#define GPIOA (&PORT->Group[0])
#define GPIOB (&PORT->Group[1])

enum GPIO_PinDirections {
	GPIO_IN = 0,
	GPIO_OUT = 1
};

enum GPIO_DriveStrength {
	NORMAL = 0,
	STRONG = 1
};

struct GPIO_PinConfig_t {
	uint8_t driveStrength:1;		// 0: normal pin drive strength; 1: "stronger" drive strength
	uint8_t enablePull:1;			// 0: internal pull resistors disabled; 1: internal pull resistors enabled. To set direction of pull, write the pin value
	uint8_t enableInputBuffer:1;	// 0: value of pin will not be sampled; 1: input value will be sampled. For some contexts, continuous sampling may also need to be enabled
	uint8_t enablePMUX:1;			// 0: pin is a normal IO pin; 1: pin is being used in alternate function mode
	uint8_t	alt_function:4;// select function to be performed by pin when in alternate function mode
};

/*******************
Configure all the things
*******************/

/* Configure settings, including direction, of a single pin in specified port */
void GPIO_ConfigurePin(PortGroup* port, uint8_t pin, enum GPIO_PinDirections direction, struct GPIO_PinConfig_t* config);
/* Configure settings, including direction, of selected pins in specified port */
void GPIO_ConfigurePort(PortGroup* port,  uint32_t pin_mask, enum GPIO_PinDirections direction, struct GPIO_PinConfig_t* config);

/* Reset configuration to default */
void GPIO_Reset(); // reverts all pins to be configured as inputs with input buffers, output buffers, and pull disabled (PULLEN, INEN, DIR all 0) with WRCONFIG
void GPIO_ResetPort(PortGroup* port); // reset one port

/*******************
Configure individual things
*******************/

/* Configure direction of single pin in specified port */
void GPIO_SetPinDirection(PortGroup* port, uint8_t pin, enum GPIO_PinDirections direction);
/* Configure direction of selected pins in specified port */
void GPIO_SetPortDirection(PortGroup* port, uint32_t pin_mask, enum GPIO_PinDirections direction);


/* Configure drive strength of pin */
void GPIO_SetDriveStrength(PortGroup* port, uint8_t pin, enum GPIO_DriveStrength strength);
/* Configure whether internal pull resistors are enabled */
void GPIO_EnablePull(PortGroup* port, uint8_t pin, _Bool enable);
/* Configure whether a pin is readable */
void GPIO_EnableInputBuffer(PortGroup* port, uint8_t pin, _Bool enable);
/* Configure whether pin is configured as alternate function mode (SERCOM, EXTINT, etc) */
void GPIO_EnablePMUX(PortGroup* port, uint8_t pin, _Bool enable);
/* Configure sampling mode of input pin */
void GPIO_EnableContinuousSampling(PortGroup* port, uint8_t pin, _Bool enable);




void GPIO_Clk_Control(bool setEnabled); // enable or disable PORT bus clock CLK_PORT_APB (default state: enabled)

uint8_t GPIO_ReadPin(PortGroup* port, uint8_t pin);
uint32_t GPIO_ReadPort(PortGroup* port);

void GPIO_WritePin(PortGroup* port, uint8_t pin, bool value);	// write value to one pin
void GPIO_WritePort(PortGroup* port, uint32_t pins, bool value); // write multiple pins with the same value
void GPIO_TogglePin(PortGroup* port, uint8_t pin);		// toggle pin on port
void GPIO_TogglePort(PortGroup* port, uint32_t pins);	// toggle multiple pins on port

// use EIC to trigger interrupts from IO lines
void GPIO_EnableExtInt(uint8_t extint_num);
void GPIO_ConfigureExtInt(uint8_t extint_num, bool enableWakeup, bool enableFiltering, uint8_t detectionMode);
void GPIO_DisableExtInt(uint8_t extint_num);

// this is handled by the PAC
//void GPIO_LockConfiguration(); // prevent further configuration

#endif /* GPIO_H_ */