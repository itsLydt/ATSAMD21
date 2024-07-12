/*
 * SPIClient_NonBlocking.c
 *
 * Created: 7/12/2024 1:12:07 PM
 * Author : lydia.klaus
 */ 

#include "sam.h"
#include "gpio.h"
#include "spi.h"

#define LED0 30		//PB30
#define SERCOM0_PAD0 8	//PA08, SPI MISO
#define SERCOM0_PAD1 9	//PA09, SPI SCK
#define SERCOM0_PAD2 10	//PA10, SPI SS
#define SERCOM0_PAD3 11	//PA11, SPI MOSI

#define BUFFER_SIZE UINT8_MAX

int main(void)
{
    GPIO_ConfigurePinAsOutput(GPIOB, LED0, false, false, -1);
    GPIO_WritePin(GPIOB, LED0, true);
    	
    /* configure SPI pins */
    GPIO_ConfigurePinAsOutput(GPIOA, SERCOM0_PAD0, false, false, 2); // AF mode C, MISO
    GPIO_ConfigurePinAsInput(GPIOA, SERCOM0_PAD1, false, true, false, 2); // SCK, pull down
    GPIO_ConfigurePinAsInput(GPIOA, SERCOM0_PAD2, false, true, true, 2); // SS, pull up
    GPIO_ConfigurePinAsInput(GPIOA, SERCOM0_PAD3, false, true, true, 2); // MOSI
	
	SPI_ClkControl(0, true, -1);
	SPI_InitClient(SERCOM0, 0, -1, 0, 0);
	NVIC_EnableIRQ(SERCOM0_IRQn);
	
	char rxBuffer[BUFFER_SIZE];
	
    while (1) 
    {
		// first byte of each transmission string is the length
		uint8_t len;
		SPI_BeginReceiveData(SERCOM0, 1);
		SPI_FinishReceiveData(SERCOM0, &len);
		// receive len bytes
		SPI_BeginReceiveData(SERCOM0, len);
		SPI_FinishReceiveData(SERCOM0, rxBuffer);
		GPIO_TogglePin(GPIOB, LED0);
				
		char* msg = "Nah I don't think I will. HELLO WORLD!";
		len = strlen(msg);
		SPI_BeginSendData(SERCOM0, &len, 1);
		SPI_BeginSendData(SERCOM0, msg, len);
		while(SPI_IsBusy());
				
		memset(rxBuffer, 0, BUFFER_SIZE);
    }
}

void SERCOM0_Handler(){
	SPI_InterruptHandler(SERCOM0);
}