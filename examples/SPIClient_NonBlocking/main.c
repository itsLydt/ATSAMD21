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
	GPIO_SetPinDirection(GPIOB, LED0, GPIO_OUT);
	GPIO_WritePin(GPIOB, LED0, true);
    	
    /* configure SPI pins */
	// MISO
	struct GPIO_PinConfig_t sercom_config = { .enablePMUX = 1, .alt_function = 2 };
	GPIO_ConfigurePin(GPIOA, SERCOM0_PAD0, GPIO_OUT, &sercom_config);
	//SCK in, SS in, MOSI
	uint32_t sercom_in = (1 << SERCOM0_PAD1) | (1 << SERCOM0_PAD2) | (1 << SERCOM0_PAD3);
	sercom_config.enablePull = 1;
	GPIO_ConfigurePort(GPIOA, sercom_in, GPIO_IN, &sercom_config);
	GPIO_WritePin(GPIOA, SERCOM0_PAD2, 1);
	GPIO_WritePin(GPIOA, SERCOM0_PAD3, 1);
	
	SPI_ClkControl(0, true, -1);
	SPI_InitClient(SERCOM0, 0, -1, 0, 0);
	NVIC_EnableIRQ(SERCOM0_IRQn);
	
	char rxBuffer[BUFFER_SIZE];
	
    while (1) 
    {
		// first byte of each transmission string is the length
		uint8_t len;
		SPI_BeginReceiveData(SERCOM0, 1);
		while(SPI_IsBusy());
		SPI_FinishReceiveData(SERCOM0, &len);
		// receive len bytes
		SPI_BeginReceiveData(SERCOM0, len);
		while(SPI_IsBusy());
		SPI_FinishReceiveData(SERCOM0, rxBuffer);
		
		GPIO_TogglePin(GPIOB, LED0);
				
		char* msg = "Nah I don't think I will. HELLO WORLD!";
		len = strlen(msg);
		SPI_BeginSendData(SERCOM0, &len, 1);
		while(SPI_IsBusy());
		SPI_BeginSendData(SERCOM0, msg, len);
		while(SPI_IsBusy());
				
		memset(rxBuffer, 0, BUFFER_SIZE);
    }
}

void SERCOM0_Handler(){
	SPI_InterruptHandler(SERCOM0);
}