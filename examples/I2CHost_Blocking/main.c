/*
 * I2CHost_Blocking.c
 *
 * Created: 7/15/2024 1:21:03 PM
 * Author : itsLydt
 */ 


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
	
    while (1) 
    {
    }
}
