/*
 * GPIO_tests.c
 *
 * Created: 7/30/2024 2:19:40 PM
 */ 

#include "gpio_tests.h"
#include "gpio.h"

#define BUFFER_SIZE 256
char msgBuffer[BUFFER_SIZE];
const uint32_t implementedPins[3] = PORT_PIN_IMPLEMENTED;

void t_pinToMask(void){
	for(int pin = 0; pin < 32; pin++){
		TEST_ASSERT_BIT_HIGH(pin, PIN_TO_MASK(pin));
	}
}

// port A does not implement pins 26 and 29
// port B does not implement pins 18, 19, 20, 21, 24, 25, 26, 27, 28, and 29
void t_setPinsAsOutput(void){
	PortGroup* port = GPIOA;
	char pinStr[3];
	for(int pin = 0; pin < 32; pin++){
		if(pin == 26 || pin == 29)
			continue;
		itoa(pin, pinStr, 10);
		TEST_ASSERT_BIT_LOW_MESSAGE(pin, port->DIR.reg, pinStr);
		GPIO_SetPinDirection(port, pin, GPIO_OUT);
		TEST_ASSERT_BIT_HIGH_MESSAGE(pin, port->DIR.reg, pinStr);
	}
	TEST_ASSERT_BITS_HIGH(implementedPins[0], port->DIR.reg);

	
	port = GPIOB;
	const int maxIndex = 10;
	int pinsToSkip[10] = { 18, 19, 20, 21, 24, 25, 26, 27, 28, 29};
	int skipIndex = 0;
	for(int pin = 0; pin < 32; pin++){
		while(skipIndex < maxIndex && pinsToSkip[skipIndex] < pin){
			++skipIndex;
		}
		if(pin == pinsToSkip[skipIndex])
			continue;
		itoa(pin, pinStr, 10);
		TEST_ASSERT_BIT_LOW_MESSAGE(pin, port->DIR.reg, pinStr);
		GPIO_SetPinDirection(port, pin, GPIO_OUT);
		TEST_ASSERT_BIT_HIGH_MESSAGE(pin, port->DIR.reg, pinStr);
	}
	
	TEST_ASSERT_BITS_HIGH(implementedPins[1], port->DIR.reg);
};

void t_setPinsAsInput(void){	
	PortGroup* port = GPIOA;
	char pinStr[3];
	for(int pin = 0; pin < 32; pin++){
		if(pin == 26 || pin == 29)
			continue;
		itoa(pin, pinStr, 10);
		TEST_ASSERT_BIT_HIGH_MESSAGE(pin, port->DIR.reg, pinStr);
		GPIO_SetPinDirection(port, pin, GPIO_IN);
		TEST_ASSERT_BIT_LOW_MESSAGE(pin, port->DIR.reg, pinStr);
	}
	TEST_ASSERT_BITS_LOW(implementedPins[0], port->DIR.reg);

	
	port = GPIOB;
	const int maxIndex = 10;
	int pinsToSkip[10] = { 18, 19, 20, 21, 24, 25, 26, 27, 28, 29};
	int skipIndex = 0;
	for(int pin = 0; pin < 32; pin++){
		while(skipIndex < maxIndex && pinsToSkip[skipIndex] < pin){
			++skipIndex;
		}
		if(pin == pinsToSkip[skipIndex])
			continue;
		itoa(pin, pinStr, 10);
		TEST_ASSERT_BIT_HIGH_MESSAGE(pin, port->DIR.reg, pinStr);
		GPIO_SetPinDirection(port, pin, GPIO_IN);
		TEST_ASSERT_BIT_LOW_MESSAGE(pin, port->DIR.reg, pinStr);
	}
	
	TEST_ASSERT_BITS_LOW(implementedPins[1], port->DIR.reg);
};

void t_setPortsAsInput(void){
	GPIO_SetPortDirection(GPIOA, 0xFFFFFFFF, GPIO_IN);
	TEST_ASSERT_BITS_LOW(implementedPins[0], GPIOA->DIR.reg);

	GPIO_SetPortDirection(GPIOB, 0xFFFFFFFF, GPIO_IN);
	TEST_ASSERT_BITS_LOW(implementedPins[1], GPIOB->DIR.reg);
}

void t_setPortsAsOutput(void){
	GPIO_SetPortDirection(GPIOA, 0xFFFFFFFF, GPIO_OUT);
	TEST_ASSERT_BITS_HIGH(implementedPins[0], GPIOA->DIR.reg);

	GPIO_SetPortDirection(GPIOB, 0xFFFFFFFF, GPIO_OUT);
	TEST_ASSERT_BITS_HIGH(implementedPins[1], GPIOB->DIR.reg);
}

void t_randomPinDirections(void){
	for(int i = 0; i < 1000; i++){
		uint8_t portNum = rand() % 2;
		uint32_t mask = rand() & implementedPins[portNum];
		enum GPIO_PinDirections direction = rand() % 2? GPIO_OUT : GPIO_IN;
		GPIO_SetPortDirection(&PORT->Group[portNum], mask, direction);
		
		switch(direction){
			case GPIO_OUT:
				TEST_ASSERT_BITS_HIGH(mask, PORT->Group[portNum].DIR.reg);
				break;
			case GPIO_IN:
				TEST_ASSERT_BITS_LOW(mask, PORT->Group[portNum].DIR.reg);
				break;
		}
	}
}

void t_resetPort(void){
	GPIO_ResetPort(GPIOB);
	uint32_t defaultDir[3] = PORT_DIR_DEFAULT_VAL;
	TEST_ASSERT_EQUAL_HEX32(defaultDir[1], REG_PORT_DIR1);
	
	uint32_t defaultOut[3] = PORT_OUT_DEFAULT_VAL;
	TEST_ASSERT_EQUAL_HEX32(defaultOut[1], REG_PORT_OUT1);
	
	for(int i = 0; i < 16; i++){
		TEST_ASSERT_EQUAL_HEX32(0, GPIOB->PMUX[i].reg);
	}
	
}
