/*
 * gpio.c
 *
 * Created: 6/26/2024 3:46:41 PM
 *  Author: lydia.klaus
 */ 

#include "gpio.h"

void GPIO_ConfigurePin(PortGroup* port, uint8_t pin, enum GPIO_PinDirections direction, struct GPIO_PinConfig_t* config){
	GPIO_ConfigurePort(port, PIN_TO_MASK(pin), direction, config);
};

void GPIO_ConfigurePort(PortGroup* port,  uint32_t pin_mask, enum GPIO_PinDirections direction, struct GPIO_PinConfig_t* config){
	GPIO_SetPortDirection(port, pin_mask, direction);
	
	uint16_t lowerPins = pin_mask & PORT_WRCONFIG_PINMASK_Msk;
	uint16_t upperPins = (pin_mask >> 16) & PORT_WRCONFIG_PINMASK_Msk;
		
	uint32_t wrconfig = PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX; // write pin config register and mux settings
	if(config->enablePMUX){
		// enable peripheral multiplexer
		wrconfig |= PORT_WRCONFIG_PMUXEN;
			
		// select peripheral function
		wrconfig |= PORT_WRCONFIG_PMUX(config->alt_function);
	}
	
	if(config->driveStrength){
		wrconfig |= PORT_WRCONFIG_DRVSTR;
	}
	if(config->enablePull){
		wrconfig |= PORT_WRCONFIG_PULLEN;
	}

	if(direction == GPIO_IN || config->enableInputBuffer){ // doesn't really make sense to have an input that isn't sampled
		wrconfig |= PORT_WRCONFIG_INEN;
	}

	// write the pin configuration
	if(lowerPins){ 	// check if any lower pins are selected
		port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_PINMASK(lowerPins);
	}
	if(upperPins){ 	// check if any upper pins are selected
		port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_PINMASK(upperPins);
	}
};

void GPIO_Reset() {
	GPIO_ResetPort(GPIOA);
	GPIO_ResetPort(GPIOB);
}

void GPIO_ResetPort(PortGroup* port){
	GPIO_SetPortDirection(port, 0xFFFFFFFF, GPIO_IN);
	port->WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PINMASK_Msk; //configure lower 16
	port->WRCONFIG.reg = PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PINMASK_Msk; //configure upper 16
}

void GPIO_Clk_Control(bool setEnabled){
	if(setEnabled){
		PM->APBBMASK.bit.PORT_ = 1; // set bit 3 to enable PORT APB clock
	}
	else {
		PM->APBBMASK.bit.PORT_ = 0; // clear bit 3 to disable PORT APB clock
	}
}

void GPIO_SetPinDirection(PortGroup* port, uint8_t pin, enum GPIO_PinDirections direction){
	GPIO_SetPortDirection(port, PIN_TO_MASK(pin), direction);
};

void GPIO_SetPortDirection(PortGroup* port, uint32_t pin_mask, enum GPIO_PinDirections direction){
	switch(direction){
		case GPIO_IN:
		// set pins to be inputs
		port->DIRCLR.reg = pin_mask;
		break;
		case GPIO_OUT:
		// set pins to be outputs
		port->DIRSET.reg = pin_mask;
		break;
	};
};

void GPIO_SetDriveStrength(PortGroup* port, uint8_t pin, enum GPIO_DriveStrength strength){
	port->PINCFG[pin].bit.DRVSTR = strength == STRONG? 1 : 0;
}

void GPIO_EnablePull(PortGroup* port, uint8_t pin, _Bool enable){
	port->PINCFG[pin].bit.PULLEN = enable? 1 : 0;	
}

void GPIO_EnableInputBuffer(PortGroup* port, uint8_t pin, _Bool enable){
	port->PINCFG[pin].bit.INEN = enable? 1 : 0;
}

void GPIO_EnablePMUX(PortGroup* port, uint8_t pin, _Bool enable){
	port->PINCFG[pin].bit.PMUXEN = enable? 1 : 0;
}

void GPIO_SetPeripheralFunction(PortGroup* port, uint8_t pin, uint8_t alt_function){
	uint32_t wrconfig = PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(alt_function);
	uint8_t shift = 0;
	
	if(pin > 15){
		wrconfig |= PORT_WRCONFIG_HWSEL;
		shift = 16;
	}
	uint32_t pin_mask = 1 << (pin - shift);
	port->WRCONFIG.reg = wrconfig | PORT_WRCONFIG_PINMASK(pin_mask);

	GPIO_EnablePMUX(port, pin, 1);
}


void GPIO_EnableContinuousSampling(PortGroup* port, uint8_t pin, _Bool enable){
	if(enable){
		port->CTRL.reg |= PIN_TO_MASK(pin);
	}
	else {
		port->CTRL.reg &= ~(PIN_TO_MASK(pin));
	}
}

void GPIO_SetConfigurationLocked(_Bool lock){
	uint32_t mask = 0x01 << 3;
	if(lock){
		PAC1->WPSET.reg = mask;	
	}
	else {
		PAC1->WPCLR.reg = mask;
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
void GPIO_WritePin(PortGroup* port, uint8_t pin, _Bool value){
	GPIO_WritePort(port, PIN_TO_MASK(pin), value);
}
/* write multiple pins with the same value */
void GPIO_WritePort(PortGroup* port, uint32_t pinMask, _Bool value){
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
	GPIO_TogglePort(port, PIN_TO_MASK(pin));
}
/* toggle multiple pins on port */ 
void GPIO_TogglePort(PortGroup* port, uint32_t pinMask){
	port->OUTTGL.reg = PORT_OUTTGL_OUTTGL(pinMask); // toggle all indicated pins
}	

/* Write values to multiple pins */
void GPIO_WriteValue(PortGroup* port, uint32_t pin_mask, uint32_t values){
	GPIO_WritePort(port, pin_mask, 0);			// clear indicated pins
	GPIO_WritePort(port, pin_mask & values, 1);	// write pins high
}

void GPIO_EnableExtInt(uint8_t extint_num){
	EIC->INTENSET.reg = EIC_INTENSET_EXTINT(1 << extint_num);;
}
// configure the EIC to trigger interrupts from IO lines
void GPIO_ConfigureExtInt(uint8_t extint_num, _Bool enableWakeup, _Bool enableFiltering, uint8_t detectionMode){

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

