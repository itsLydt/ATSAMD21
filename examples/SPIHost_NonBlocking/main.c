/*
 * SPIHost_NonBlocking.c
 *
 * Created: 7/11/2024 9:44:33 AM
 * Author : lydia.klaus
 */ 


#include "sam.h"

#include "gpio.h"
#include "spi.h"

#define BUTTON0 15	//PA15
#define LED0 30		//PB30

#define SERCOM0_PAD0 8	//PA08, SPI MOSI
#define SERCOM0_PAD1 9	//PA09, SPI SCK
#define SERCOM0_PAD2 10	//PA10, SPI SS
#define SERCOM0_PAD3 11	//PA11, SPI MISO

bool flag = false;
int main(void)
{
	/* Configure the user LED */
	GPIO_SetPinDirection(GPIOB, LED0, GPIO_OUT);
	GPIO_WritePin(GPIOB, LED0, true);
	
	/* configure SPI pins */
	uint32_t sercom_out = (1 << SERCOM0_PAD0) | (1 << SERCOM0_PAD1) | (1 << SERCOM0_PAD2);
	struct GPIO_PinConfig_t sercom_config = { .enablePMUX = 1, .alt_function = 2 };
	GPIO_ConfigurePort(GPIOA, sercom_out, GPIO_OUT, &sercom_config);
	GPIO_ConfigurePin(GPIOA, SERCOM0_PAD3, GPIO_IN, &sercom_config);
	/* configure the EIC clocks */
	// CLK_EIC_APB - on by default
	// enable GCLK_EIC
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_EIC;
	
	// configure button (SW0) as an input, select AF function A (EXTINT[15])
	struct GPIO_PinConfig_t btn_config = { .enablePull = 1, .enablePMUX = 1, .alt_function = 0};
	GPIO_ConfigurePin(GPIOA, BUTTON0, GPIO_IN, &btn_config);
	GPIO_WritePin(GPIOA, BUTTON0, 1);	// enable pull up
	
	// write the EIC configuration registers (EVCTRL, WAKEUP, CONFIGy) and enable the EIC and EIC interrupts
	GPIO_ConfigureExtInt(15, true, false, 2);
	GPIO_EnableExtInt(15);

	/* enable generic clock generator 1 to use for SPI clock */
	GCLK->GENDIV.reg = GCLK_GENDIV_DIV(16) | GCLK_GENDIV_ID(1); // set divider of GEN1 to 16
	GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_ID(1);
	SPI_ClkControl(0, true, 1);
	SPI_InitHost(SERCOM0, 0, 0, true);
	
	// to use the non-blocking calls, make sure to configure the NVIC
	NVIC_EnableIRQ(SERCOM0_IRQn);
	
	char* msg = "Have you considered writing something interesting instead of the boilerplate text everyone uses for everything";
	uint32_t delay = 10000;
	char response[UINT8_MAX];
    while (1){
		while(!flag);			// wait for button click to occur

		GPIO_WritePin(GPIOB, LED0, false);
	
		// send length of packet so client knows how much to read
		uint8_t len = strlen(msg);
		SPI_BeginSendData(SERCOM0, &len, 1);
		while(SPI_IsBusy());
		// send msg
		SPI_BeginSendData(SERCOM0, msg, strlen(msg));
		while(SPI_IsBusy());
		
		// wait
		for(int i = 0; i < delay; i++); //wait for client to be ready

		// get length of return message
		SPI_BeginReceiveData(SERCOM0, 1);
		while(SPI_IsBusy());
		SPI_FinishReceiveData(SERCOM0, &len);
		// get the response
		SPI_BeginReceiveData(SERCOM0, len);
		while(SPI_IsBusy());
		SPI_FinishReceiveData(SERCOM0, &response);
	
		GPIO_WritePin(GPIOB, LED0, true);
		flag = false;
	}
}


void SERCOM0_Handler(){
	SPI_InterruptHandler(SERCOM0);
}

void EIC_Handler(){ //triggered on EXTINT[15] falling edge
	//temporarily disable the interrupt
	GPIO_DisableExtInt(15);
	// clear the interrupt
	EIC->INTFLAG.reg = EIC_INTENSET_EXTINT15;
	flag = true;
	
	// re-enable
	GPIO_EnableExtInt(15);
}