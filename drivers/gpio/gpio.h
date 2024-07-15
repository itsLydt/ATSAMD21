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

void GPIO_ConfigurePinAsOutput(PortGroup* port, uint8_t pin, bool enableRead, bool driveStrength, int8_t function);
void GPIO_ConfigurePortAsOutput(PortGroup* port, uint32_t pinMask, bool enableRead, bool driveStrength, int8_t function);
void GPIO_ConfigurePinAsInput(PortGroup* port, uint8_t pin, bool continousSampling, bool enablePull, bool pullDirection, int8_t function);
void GPIO_ConfigurePortAsInput(PortGroup* port, uint32_t pinMask, bool continousSampling, bool enablePull, bool pullDirection, int8_t function);

void GPIO_Reset(); // all pins configured as inputs with input buffers, output buffers, and pull disabled (PULLEN, INEN, DIR all 0) with WRCONFIG
void GPIO_ResetPort(PortGroup* port); // reset one port

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