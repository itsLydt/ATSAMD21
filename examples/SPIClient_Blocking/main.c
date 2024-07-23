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

#define BUFFER_SIZE UINT8_MAX
int main(void)
{
	GPIO_SetPinDirection(GPIOB, LED0, GPIO_OUT);
	GPIO_WritePin(GPIOB, LED0, true);
	
	/* configure SPI pins */
	GPIO_SetPeripheralFunction(GPIOA, SERCOM0_PAD0, 2); //MISO
	GPIO_SetPeripheralFunction(GPIOA, SERCOM0_PAD1, 2); //SCK in
	GPIO_SetPeripheralFunction(GPIOA, SERCOM0_PAD2, 2); //SS in
	GPIO_SetPeripheralFunction(GPIOA, SERCOM0_PAD3, 2); //MOSI

	SPI_ClkControl(0, true, -1);
	SPI_InitClient(SERCOM0, 0, -1, 0, 0);
	
	char rxBuffer[BUFFER_SIZE];

	while (1) 
    {
		// first byte of each transmission string is the length
		uint8_t len;
		SPI_ReceiveData(SERCOM0, &len, 1);
		// receive len bytes
		SPI_ReceiveData(SERCOM0, rxBuffer, len);
		GPIO_TogglePin(GPIOB, LED0);
		
		char* msg = "Nah I don't think I will. HELLO WORLD!";
		len = strlen(msg);
		SPI_SendData(SERCOM0, &len, 0, 1);
		SPI_SendData(SERCOM0, msg, 0, len);
		
		memset(rxBuffer, 0, BUFFER_SIZE);
    }
}
