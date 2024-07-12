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
	GPIO_ConfigurePinAsOutput(GPIOB, LED0, true, false, -1);
	GPIO_WritePin(GPIOB, LED0, true);
	
	/* configure the EIC */
	// enable CLK_EIC_APB - on by default
	// enable GCLK_EIC
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_EIC;
	
	// configure button (SW0) as an input, select AF function A (EXTINT[15])
	GPIO_ConfigurePinAsInput(GPIOA, BUTTON0, false, true, true, 0);
	
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