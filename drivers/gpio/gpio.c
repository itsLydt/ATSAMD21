/*
 * gpio.c
 *
 * Created: 6/26/2024 3:46:41 PM
 *  Author: lydia.klaus
 */ 

#include "gpio.h"

/* set all pins as inputs with input buffers, output buffers, and pull disabled (PULLEN, INEN, DIR all 0), no peripheral functions */
void GPIO_Reset() {
	GPIO_ResetPort(GPIOA);
	GPIO_ResetPort(GPIOB);
}

/* set pins of given port as inputs with input buffers, output buffers, and pull disabled (PULLEN, INEN, DIR all 0), no peripheral functions */
void GPIO_ResetPort(PortGroup* port){
	port->WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PINMASK_Msk; //configure lower 16
	port->WRCONFIG.reg = PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PINMASK_Msk; //configure upper 16
}

 /* enable or disable PORT bus clock CLK_PORT_APB (default state: enabled) */
void GPIO_Clk_Control(bool setEnabled){
	if(setEnabled){
		PM->APBBMASK.bit.PORT_ = 1; // set bit 3 to enable PORT APB clock
	}
	else {
		PM->APBBMASK.bit.PORT_ = 0; // clear bit 3 to disable PORT APB clock
	}
}

// enableRead: 1 to enable input buffer when using pin as output
// driveStrength: 0 = normal, 1 = excedrin extra strength formula
// function: select peripheral function. Set to -1 to disable PMUX and use as normal GPIO 
void GPIO_ConfigurePinAsOutput(PortGroup* port, uint8_t pin, bool enableRead, bool driveStrength, int8_t function){
	uint32_t pinMask = 0x01 << pin; // convert pin number to mask
	GPIO_ConfigurePortAsOutput(port, pinMask, enableRead, driveStrength, function);
}

void GPIO_ConfigurePortAsOutput(PortGroup* port, uint32_t pinMask, bool enableRead, bool driveStrength, int8_t function){
	
	// set pin direction to output
	port->DIRSET.reg = pinMask;
	
	uint16_t lowerPins = pinMask & PORT_WRCONFIG_PINMASK_Msk;
	uint16_t upperPins = (pinMask >> 16) & PORT_WRCONFIG_PINMASK_Msk;
	
	uint32_t wrconfig = PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX;

	if(driveStrength)
		wrconfig |= PORT_WRCONFIG_DRVSTR;
	if(enableRead)
		wrconfig |= PORT_WRCONFIG_INEN;

	if(function != -1){
		// enable peripheral multiplexer
		wrconfig |= PORT_WRCONFIG_PMUXEN;
		// select peripheral function
		wrconfig |= PORT_WRCONFIG_PMUX(function);
	}
	
	// write the pin configuration
	if(lowerPins){ 	// check if any lower pins are selected
		port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_PINMASK(lowerPins);
	}
	if(upperPins){ 	// check if any upper pins are selected
		port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_PINMASK(upperPins);
	}
}

// continuousSampling: sample continuously for use with IOBUS, increased power consumption
// enablePull: 1 to enable pull-up/pull-down
// pullDirection: 1 = high, 0 = low
// function: select peripheral function. Set to -1 to disable PMUX and use as normal GPIO
void GPIO_ConfigurePinAsInput(PortGroup* port, uint8_t pin, bool continousSampling, bool enablePull, bool pullDirection, int8_t function){
	uint32_t pinMask = 0x01 << pin; // convert pin number to mask
	GPIO_ConfigurePortAsInput(port, pinMask, continousSampling, enablePull, pullDirection, function);
}

void GPIO_ConfigurePortAsInput(PortGroup* port, uint32_t pinMask, bool continousSampling, bool enablePull, bool pullDirection, int8_t function){
	// set pin direction to input
	port->DIRCLR.reg = pinMask;
	
	uint16_t lowerPins = pinMask & PORT_WRCONFIG_PINMASK_Msk;
	uint16_t upperPins = (pinMask >> 16) & PORT_WRCONFIG_PINMASK_Msk;
		
	uint32_t wrconfig = PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_INEN;
	if(enablePull){
		wrconfig |= PORT_WRCONFIG_PULLEN;
			
		//TODO: does the wrconfig value need to be written first?
		GPIO_WritePort(port, pinMask, pullDirection);
	}
	
	if(function != -1){
		// enable peripheral multiplexer
		wrconfig |= PORT_WRCONFIG_PMUXEN;
		// select peripheral function
		wrconfig |= PORT_WRCONFIG_PMUX(function);
	}
	
	// write the pin configuration
	if(lowerPins){ 	// check if any lower pins are selected
		wrconfig |= PORT_WRCONFIG_PINMASK(lowerPins);
		port->WRCONFIG.reg = wrconfig;
	}
	if(upperPins){ 	// check if any upper pins are selected
		port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_PINMASK(upperPins);
	}
	
	// set the sampling mode
	if(continousSampling){
		port->CTRL.reg |= pinMask;
	}
	else {
		port->CTRL.reg &= ~pinMask;
	}
} 

/* read the state of the pin */
uint8_t GPIO_ReadPin(PortGroup* port, uint8_t pin){
	return (GPIO_ReadPort(port) >> pin) & 0x01;
}
/* read the state of the port */
uint32_t GPIO_ReadPort(PortGroup* port){
	return port->IN.reg;
}

/* write value to one pin. If the pin is an input, this will set the pull direction (if enabled) */
void GPIO_WritePin(PortGroup* port, uint8_t pin, bool value){
	uint32_t pinMask = 0x01 << pin; // convert pin number to mask
	GPIO_WritePort(port, pinMask, value);
}
/* write multiple pins with the same value */
void GPIO_WritePort(PortGroup* port, uint32_t pinMask, bool value){
	if(value){
		port->OUTSET.reg = PORT_OUTSET_OUTSET(pinMask); // set all indicated pins high
	}
	else {
		port->OUTCLR.reg = PORT_OUTCLR_OUTCLR(pinMask); // set all indicated pins low
	}
}

/* Toggle will also affect input pins with pull resistors enabled */
/* toggle pin state */
void GPIO_TogglePin(PortGroup* port, uint8_t pin){
	uint32_t pinMask = 0x01 << pin; // convert pin number to mask
	GPIO_TogglePort(port, pinMask);
}
/* toggle multiple pins on port */ 
void GPIO_TogglePort(PortGroup* port, uint32_t pinMask){
	port->OUTTGL.reg = PORT_OUTTGL_OUTTGL(pinMask); // toggle all indicated pins
}	

void GPIO_EnableExtInt(uint8_t extint_num){
	EIC->INTENSET.reg = EIC_INTENSET_EXTINT(1 << extint_num);;
}
// configure the EIC to trigger interrupts from IO lines
void GPIO_ConfigureExtInt(uint8_t extint_num, bool enableWakeup, bool enableFiltering, uint8_t detectionMode){

	if(enableWakeup){
		EIC->WAKEUP.reg |= EIC_WAKEUP_WAKEUPEN(1 << extint_num);
	}
	else {
		EIC->WAKEUP.reg &= ~EIC_WAKEUP_WAKEUPEN(1 << extint_num);
	}
	
	volatile uint32_t* config_reg = extint_num > 7? &REG_EIC_CONFIG1 : &REG_EIC_CONFIG0;
	uint8_t config_pos = extint_num > 7? extint_num - 8 : extint_num;
	config_pos = config_pos * 4;
	*config_reg &= ~(0xF << config_pos); //clear four bits for selected IRQ
	
	uint8_t config_value = detectionMode;
	if(enableFiltering){
		config_value |= 0x08;
	}
	
	*config_reg |= (config_value << config_pos);
	
	// make sure the EIC is enabled
	EIC->CTRL.bit.ENABLE = 1;
	while(EIC->STATUS.bit.SYNCBUSY);
	
	//make sure the NVIC is allowing EIC interrupts
	NVIC_EnableIRQ(EIC_IRQn);
}

void GPIO_DisableExtInt(uint8_t extint_num){
	EIC->INTENCLR.reg = EIC_INTENCLR_EXTINT(1 << extint_num);
}

