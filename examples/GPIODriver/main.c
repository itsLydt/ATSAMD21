/*
 * GPIODriver.c
 *
 * Created: 6/24/2024 5:02:22 PM
 * Author : itsLydt
 */ 


#include "sam.h"

#include "gpio.h"

#define LED0 30		//PB30
#define BUTTON0 15	//PA15
int main(void)
{
	// configure LED0
	GPIO_SetPinDirection(GPIOB, LED0, GPIO_OUT);
	GPIO_WritePin(GPIOB, LED0, 1);
	
	/* configure the EIC */
	// enable CLK_EIC_APB - on by default
	// enable GCLK_EIC
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_EIC;
	
	// configure button (SW0) as an input, enable pull, and select AF function A (EXTINT[15])	
	struct GPIO_PinConfig_t sw0_config = { .enablePull = 1, .enablePMUX = 1, .alt_function = 0};
	GPIO_ConfigurePin(GPIOA, BUTTON0, GPIO_IN, &sw0_config);
	// set pull direction high
	GPIO_WritePin(GPIOA, BUTTON0, 1);
	
	// write the EIC configuration registers (EVCTRL, WAKEUP, CONFIGy) and enable the EIC and EIC interrupts
	GPIO_ConfigureExtInt(15, true, false, 2);
	GPIO_EnableExtInt(15);

	while (1);
}

void EIC_Handler(){
	//triggered on EXTINT[15] falling edge
	
	//temporarily disable the interrupt
	GPIO_DisableExtInt(15);
	EIC->INTFLAG.reg = EIC_INTENSET_EXTINT15;
	uint32_t delay = 50000;
	GPIO_TogglePin(GPIOB, LED0);
	for(int i = 0; i < delay; i++);
	
	// re-enable
	GPIO_EnableExtInt(15);
}