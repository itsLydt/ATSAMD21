/*
 * SPIDriver.c
 *
 * Created: 7/2/2024 12:00:43 PM
 * Author : itsLydt
 */ 

#include <string.h>


#include "sam.h"

#include "gpio.h"
#include "spi.h"

#define BUTTON0 15	//PA15
#define LED0 30		//PB30
#define SERCOM0_PAD0 8	//PA08, SPI MOSI
#define SERCOM0_PAD1 9	//PA09, SPI SCK
#define SERCOM0_PAD2 10	//PA10, SPI SS
#define SERCOM0_PAD3 11	//PA11, SPI MISO

#define BUFFER_SIZE 256
char rxBuffer[BUFFER_SIZE];

int main(void)
{
	GPIO_ConfigurePinAsOutput(GPIOB, LED0, false, false, -1);
	GPIO_WritePin(GPIOB, LED0, true);
	
	/* configure SPI pins */
	GPIO_ConfigurePinAsOutput(GPIOA, SERCOM0_PAD0, false, false, 2); // AF mode C MOSI
	GPIO_ConfigurePinAsOutput(GPIOA, SERCOM0_PAD1, false, false, 2); // SCK
	GPIO_ConfigurePinAsOutput(GPIOA, SERCOM0_PAD2, false, false, 2); // SS
	GPIO_ConfigurePinAsInput(GPIOA, SERCOM0_PAD3, false, false, false, 2); //MISO

	/* configure the EIC clocks */
	// CLK_EIC_APB - on by default
	GCLK->GENDIV.reg = GCLK_GENDIV_DIV(16) | GCLK_GENDIV_ID(1);
	// enable GCLK_EIC
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_EIC;
	// configure button (SW0) as an input, select AF function A (EXTINT[15])
	GPIO_ConfigurePinAsInput(GPIOA, BUTTON0, false, true, true, 0);
		
	// write the EIC configuration registers (EVCTRL, WAKEUP, CONFIGy) and enable the EIC and EIC interrupts
	GPIO_ConfigureExtInt(15, true, false, 2);
	GPIO_EnableExtInt(15);
	
	// enable GEN1
	GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_ID(1);
	SPI_ClkControl(0, true, 1);
	SPI_InitMaster(SERCOM0, 0, 0, true);
	
    while (1);
}

void EIC_Handler(){ //triggered on EXTINT[15] falling edge

	//temporarily disable the interrupt
	GPIO_DisableExtInt(15);
	// clear the interrupt
	EIC->INTFLAG.reg = EIC_INTENSET_EXTINT15;

	char* msg = "Have you considered writing something interesting instead of the boilerplate text everyone uses for everything";
	uint32_t delay = 50000;

	GPIO_WritePin(GPIOB, LED0, false);
	
	// send length of packet so client knows how much to read
	uint8_t len = strlen(msg);
	SPI_SendData(SERCOM0, &len, &rxBuffer, 1);

	// send msg
	SPI_SendData(SERCOM0, msg, &rxBuffer, strlen(msg));
	for(int i = 0; i < delay; i++);
	GPIO_WritePin(GPIOB, LED0, true);

	// re-enable
	GPIO_EnableExtInt(15);
}