/*
 * SPIClient.c
 *
 * Created: 7/3/2024 4:03:16 PM
 * Author : itsLydt
 */ 

#include <string.h>

#include "sam.h"
#include "gpio.h"
#include "spi.h"

#define LED0 30		//PB30
#define SERCOM0_PAD0 8	//PA08, SPI MISO
#define SERCOM0_PAD1 9	//PA09, SPI SCK
#define SERCOM0_PAD2 10	//PA10, SPI SS
#define SERCOM0_PAD3 11	//PA11, SPI MOSI

#define BUFFER_SIZE 256
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
	SPI_InitSlave(SERCOM0, 0, -1, 0, 0);
	
	char rxBuffer[BUFFER_SIZE];

	while (1) 
    {
		int received = SPI_ReceiveData(SERCOM0, rxBuffer, BUFFER_SIZE);
		if(received > 0){
			GPIO_TogglePin(GPIOB, LED0);
		}
		memset(rxBuffer, 0, BUFFER_SIZE);
    }
}
