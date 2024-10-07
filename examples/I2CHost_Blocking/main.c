/*
 * I2CHost_Blocking.c
 *
 * Created: 7/15/2024 1:21:03 PM
 * Author : itsLydt
 */ 

#include <string.h>

#include "sam.h"

#include "gpio.h"
#include "i2c.h"

#define BUTTON0 15	//PA15
#define LED0 30		//PB30

#define I2C_SDA 8	//PA08
#define I2C_SCL 9	//PA09


#define BUFFER_SIZE UINT8_MAX
char rxBuffer[BUFFER_SIZE];

int main(void)
{
    /* Configure the user LED */
    GPIO_SetPinDirection(GPIOB, LED0, GPIO_OUT);
    GPIO_WritePin(GPIOB, LED0, 1);
	
	/* configure I2C pins */
	GPIO_SetPeripheralFunction(GPIOA, I2C_SDA, 2);
	GPIO_SetPeripheralFunction(GPIOA, I2C_SCL, 2);
	
	/* configure the EIC clocks */
	// CLK_EIC_APB - on by default
	// enable GCLK_EIC, set to generic clock generator 0
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_EIC;

	// configure button (SW0) as an input, select AF function A (EXTINT[15])
	struct GPIO_PinConfig_t btn_config = { .enablePull = 1, .enablePMUX = 1, .alt_function = 0};
	GPIO_ConfigurePin(GPIOA, BUTTON0, GPIO_IN, &btn_config);
	GPIO_WritePin(GPIOA, BUTTON0, 1);	// set pull up
	
	// write the EIC configuration registers (EVCTRL, WAKEUP, CONFIGy) and enable the EIC and EIC interrupts
	GPIO_ConfigureExtInt(15, true, false, 2);
	GPIO_EnableExtInt(15);
	
	I2C_ClkControl(0, true, 0, -1); // enable SERCOM0 bus clock, set SERCOM0 core clock to GEN0
	uint8_t baud, baudlow;
	if(I2C_TryCalcBaud(1000, 50, &baud, &baudlow)){
		I2C_InitHost(SERCOM0, 0, 0, 3, true, baud, baudlow);
	}
	else {
		// target baud rate not possible, etc
		return -1;
	}
	
    while (1);
}

void EIC_Handler(){
	//temporarily disable the interrupt
	GPIO_DisableExtInt(15);
	// clear the interrupt
	EIC->INTFLAG.reg = EIC_INTENSET_EXTINT15;
	
	uint8_t addr = 10;
	char* msg = "An extremely important sequence of bytes";
	uint32_t delay = 50000;
	
	GPIO_WritePin(GPIOB, LED0, false);
	I2CHost_SendData(SERCOM0, addr, (unsigned char*)msg, strlen(msg));
	
	// wait
	for(int i = 0; i < delay; i++);
	
	GPIO_WritePin(GPIOB, LED0, true);
	// re-enable
	GPIO_EnableExtInt(15);
}